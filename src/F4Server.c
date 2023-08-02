#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "F4Server.h"
#include "F4lib.h"

void print_usage() {
    info(
        "Usage: ./F4Server {rows} {cols} {player1_symbol} {player2_symbol}\n");
}

void check_args(int argc, char *argv[]) {
    if (argc != 5) {
        print_usage();
        exit(-1);
    }

    int row = atoi(argv[1]);
    if (row == 0) {
        info("'%s' is not a number\n", argv[1]);

        print_usage();
        exit(-1);
    }

    int col = atoi(argv[2]);
    if (col == 0) {
        info("'%s' is not a number\n", argv[2]);

        print_usage();
        exit(-1);
    }

    if (row < 5 || col < 5) {
        info("row and col must both be greater than or equal to 5\n");
        print_usage();
        exit(-1);
    }

    if (strlen(argv[3]) != 1 || strlen(argv[4]) != 1) {
        info("player symbol must be 1 character only\n");
        print_usage();
        exit(-1);
    }
    char player1_symbol = *argv[3];
    char player2_symbol = *argv[4];

    if ((int)player1_symbol == (int)player2_symbol) {
        info("player1_symbol must be different from player2_symbol\n");
        print_usage();
        exit(-1);
    }
}

void err_exit(const char *msg) {
    perror(msg);
    clear_resources();
    exit(1);
}

void clear_resources() {
    info("\n");

    // Shared Memory for board
    info("cleaning board\n");
    shmdt(board);
    shmctl(board_shmid, IPC_RMID, NULL);

    // Semaphores
    info("cleaning semaphores\n");
    for (int i = 0; i < 2; ++i) {
        semctl(game->players[i].player_semid, 0, IPC_RMID);
    }

    // Shared Memory for game data
    info("cleaning game data\n");
    shmdt(game);
    shmctl(game_shmid, IPC_RMID, NULL);

    // Shared Memory for client data
    info("cleaning client data\n");
    shmctl(client_data_shmid, IPC_RMID, NULL);
    shmdt(client_data);

    // Queues
    info("cleaning queues\n");
    msgctl(game_msqid, IPC_RMID, NULL);
    msgctl(new_connection_msqid, IPC_RMID, NULL);
}

bool out_of_bounds(int i, int j) {
    int rows = game->board_rows;
    int cols = game->board_cols;
    return j < 0 || j > cols - 1 || i < 0 || i > rows - 1;
}

int count_in_direction(char *B, char symbol, int dx, int dy, int i, int j) {
    int rows = game->board_rows;
    int cols = game->board_cols;
    int count = 0;
    while (!out_of_bounds(i, j) && B[(i * cols) + j] == symbol) {
        i += dy;
        j += dx;
        count++;
    }
    return count;
}

// cerco coppie uguale nella direzione opposta e
// aggiorno opportunamente i e j
//
// esempio:
//
//    │
//    ▼
//    X
// ┌─┬─┬─┬─┬─┐
// │ │ │ │ │ │
// ├─────────┤
// │ │ │ │X│ │
// ├─────────┤
// │ │ │X│O│ │
// ├─────────┤
// │ │ │O│O│ │
// ├─────────┤
// │X│O│O│O│ │
// └─┴─┴─┴─┴─┘
bool game_over(char *B, int i, int j) {
    int rows = game->board_rows;
    int cols = game->board_cols;
    bool won = false;
    char symbol = B[(i * cols) + j];

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            int count = count_in_direction(B, symbol, dx, dy, i, j) +
                        count_in_direction(B, symbol, -dx, -dy, i - dy, j - dx);
            won |= count >= 4;
            if (won) {
                goto exit;
            }
        }
    }

exit:
    return won;
}

enum GameMsgType insert_symbol(char *B, char symbol, int j) {
    int rows = game->board_rows;
    int cols = game->board_cols;

    // colonna non valida
    if (out_of_bounds(0, j)) {
        return COLINVALID;
    }

    int i = rows - 1;
    // trova la prima riga libera nella colonna 'j'
    while (B[(i * cols) + j] != ' ' && --i >= 0)
        ;
    if (i < 0) {
        return COLFULL;
    }

    B[(i * cols) + j] = symbol;

    return game_over(B, i, j) ? GAMEOVER : CONTINUE;
}

void siginthandler(int code) { clear_resources(); }

void init_player(char *name, NewConnectionMsg msg, int num_player){
    strcpy(game->players[game->num_players - 1].name, name);
    
    // Save SemaphoreID
    int sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    // Save id to game
    game->players[game->num_players - 1].player_semid = sem;
    // Send semaphore id to client
    msg.mdata.res.player_semid = sem;
    // initialize semaphore to 0
    semctl(sem, 0, SETVAL, 0);

    // Set user token
    game->players[game->num_players - 1].token = game->tokens[num_player]; // first player

    // Set other info
    msg.mtype = NewConnectionReq;
    // Send game info to client
    msg.mdata.res.game_shmid = game_shmid;
    msg.mdata.res.game_msqid = game_msqid;
}

void init_player_error(NewConnectionMsg msg, enum NewConnectionMsgType error){
    msg.mtype = error;
    msg.mdata.res.game_msqid = -1;
    msg.mdata.res.game_shmid = -1;
    msg.mdata.res.player_semid = -1;
}

void accept_conn() {
    while (game->num_players < 2) {
        game->num_players++;

        NewConnectionMsg new_connection_msg;

        info("waiting for new connections\n");
        new_connection_msgrcv(new_connection_msqid, &new_connection_msg);
        
        char *name = new_connection_msg.mdata.req.name;
        info("name = %s, num_players = %d\n", name, game->num_players);
        if (game->num_players == 1) {
            init_player(name, new_connection_msg, 0);

            // Send message to client
            new_connection_msgsnd(new_connection_msqid, &new_connection_msg);

        } else {
            strcpy(game->players[game->num_players - 1].name, name);

            if (strcmp(game->players[game->num_players - 1].name, game->players[game->num_players - 2].name) == 0) {
                game->num_players--;

                init_player_error(new_connection_msg, NameAlreadyTaken);

                new_connection_msgsnd(new_connection_msqid, &new_connection_msg);
            } else {
                init_player(name, new_connection_msg, 1);

                // Send message to client
                new_connection_msgsnd(new_connection_msqid, &new_connection_msg);
            }
        }
    }
}

void init_board(char *B) {
    for (int i = 0; i < game->board_rows; ++i) {
        for (int j = 0; j < game->board_cols; ++j) {
            B[(i * game->board_cols) + j] = ' ';
        }
    }
}

int main(int argc, char **argv) {
    signal(SIGINT, siginthandler);

    // Check arguments
    check_args(argc, argv);

    // init queues
    new_connection_msqid = msgget(NEW_CONNECTION_MSGKEY, IPC_CREAT | S_IRUSR | S_IWUSR);
    game_msqid = msgget(GAME_MSGKEY, IPC_CREAT | S_IRUSR | S_IWUSR);

    // Init shared memory
    game_shmid = shmget(GAME_SHMKEY, sizeof(Game), IPC_CREAT | S_IRUSR | S_IWUSR);
    game = shmat(game_shmid, NULL, 0);
    
    // Config game struct
    game->board_rows = atoi(argv[1]);
    game->board_cols = atoi(argv[2]);
    game->num_players = 0;
    game->turn = 0;
    game->tokens[0] = argv[3][0];
    game->tokens[1] = argv[4][0];
    memset(game->winner, 0, sizeof(char) * 32); // Set winner as null as char[32]
    game->game_over = false;

    // Init shared board
    board_shmid = shmget(BOARD_SHMKEY, sizeof(char) * game->board_rows * game->board_cols, IPC_CREAT | S_IRUSR | S_IWUSR);
    board = shmat(board_shmid, NULL, 0);

    // Config game
    game->board_shmid = board_shmid;

    // // initialize global variables
    // client_data_shmid = shmget(CLIENT_DATA_SHMKEY, sizeof(ClientData), IPC_CREAT | S_IRUSR | S_IWUSR);
    // client_data = shmat(client_data_shmid, NULL, SHM_RDONLY);

    // initialize board array
    init_board(board);

    // Connections
    accept_conn(); // Players are blocked

    info("Game Started!\n");
    
    do {
        info("Current turn: %d\n", game->turn);
        
        // Get current player info
        int curr_player_semid = game->players[game->turn].player_semid;
        char *username = game->players[game->turn].name;

        info("Unlocking player: %d\n", curr_player_semid);
        // Unlock other player's semaphore
        sem_release(curr_player_semid);
        
        // Game loop
        GameMsg msg;
        if (game_msgrcv(game_msqid, &msg) != -1) {
            switch (msg.mtype) {
                case MOVE: {
                    MoveData md = msg.mdata;
                    msg.mtype = insert_symbol(board, md.move.token, md.move.col);

                    // Game over
                    if (msg.mtype == GAMEOVER) {
                        info("%s is the winner!\n", username);
                        // Copy user name to game->winner
                        strcpy(game->winner, username);
                        // Set global game over
                        game->game_over = true;

                        // Send game over message
                        game_msgsnd(game_msqid, &msg);
                        info("Game over!\n");

                        break;
                    }

                    // Handle user turns
                    if (msg.mtype == CONTINUE) {
                        info("move result: %lu\n", msg.mtype);
                        game->turn = (game->turn + 1) % 2;
                    }
                    game_msgsnd(game_msqid, &msg);

                }
                case DISCONNECT: {
                    // accept_connections();
                }
                default: {
                    break;
                }
            }
        }
    } while(!game->game_over);
}
