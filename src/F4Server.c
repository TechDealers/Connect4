#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#include "F4Server.h"
#include "F4lib.h"

void print_usage() {
    printf(
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
    // when interrupted by <Ctrl-c>, don't clear resources
    // as that would be handled by `exithandler`
    if (errno == EINTR) {
        return;
    }
    perror(msg);
    clear_resources();
    exit(1);
}

void exithandler(int sig) {
    // Announce to players game will shutdown
    for (int i = 0; i < 2; ++i) {
        Player player = game->players[i];
        if (kill(player.pid, SIGUSR1) == -1) {
            info("Failed to send signal\n");
        }
        semctl(player.semid, 0, IPC_RMID);
    }
    clear_resources();
    exit(0);
}

void warnhandler(int sig) {
    info("Press Ctrl+C again to terminate the program\n");
    signal(sig, exithandler);
}
void clear_resources() {
    printf("\n");

    // Shared Memory for board
    info("cleaning board shm\n");

    if (shmctl(game->board_shmid, IPC_RMID, NULL) == -1) {
        info("Error cleaning board\n");
    }
    shmdt(board);

    // Shared Memory for game data
    info("cleaning game shm\n");

    if (shmctl(game_shmid, IPC_RMID, NULL) == -1) {
        info("Error cleaning game\n");
    }
    shmdt(game);

    // Queues
    info("cleaning msg queues\n");
    if (msgctl(game_msqid, IPC_RMID, NULL) == -1) {
        info("Error cleaning game msqid\n");
    }
    if (msgctl(new_connection_msqid, IPC_RMID, NULL) == -1) {
        info("Error cleaning new connection msqid\n");
    }

    info("Finished cleaning\n");
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

bool game_tie(char *B) {
    bool tied = true;

    for (int j = 0; j < game->board_cols; ++j) {
        if (B[j] == ' ') {
            tied = false;
            break;
        }
    }
    return tied;
}

enum GameMsgType insert_symbol(char *B, char symbol, int j) {
    int rows = game->board_rows;
    int cols = game->board_cols;

    // colonna non valida
    if (out_of_bounds(0, j)) {
        return ColInvalid;
    }

    int i = rows - 1;
    // trova la prima riga libera nella colonna 'j'
    while (B[(i * cols) + j] != ' ' && --i >= 0)
        ;
    if (i < 0) {
        return ColFull;
    }

    B[(i * cols) + j] = symbol;

    return game_over(B, i, j) ? GameOver : game_tie(B) ? GameTie : Continue;
}

void init_player(NewConnectionMsg *msg, int player_id) {
    game->num_players++;
    strcpy(game->players[player_id].name, msg->mdata.req.name);

    // initialize semaphore to 0
    int sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semctl(sem, 0, SETVAL, 0);
    // Save semid to game
    game->players[player_id].semid = sem;
    // Save player's pid
    game->players[player_id].pid = msg->mdata.req.pid;

    msg->mtype = NewConnection;
    // Send unique player id to client
    msg->mdata.res.player_id = player_id;
    // Send game info to client
    msg->mdata.res.game_shmid = game_shmid;
    msg->mdata.res.game_msqid = game_msqid;
    msg->mdata.res.timeout = 30;
}

void init_player_error(NewConnectionMsg *msg, enum NewConnectionMsgType error) {
    msg->mdata.res.game_msqid = -1;
    msg->mdata.res.game_shmid = -1;
    msg->mdata.res.player_id = -1;
    msg->mtype = error;
}

void accept_conn() {
    while (game->num_players < 2) {
        NewConnectionMsg new_connection_msg;

        info("waiting for new connections\n");
        new_connection_msgrcv(new_connection_msqid, &new_connection_msg);

        if (game->num_players == 0) {
            init_player(&new_connection_msg, 0);

            // Send message to client
            new_connection_msgsnd(new_connection_msqid, &new_connection_msg);

        } else {
            if (strcmp(game->players[0].name,
                       new_connection_msg.mdata.req.name) == 0) {
                init_player_error(&new_connection_msg, NameAlreadyTaken);

                new_connection_msgsnd(new_connection_msqid,
                                      &new_connection_msg);
            } else {
                init_player(&new_connection_msg, 1);

                // Send message to client
                new_connection_msgsnd(new_connection_msqid,
                                      &new_connection_msg);
            }
        }
        info("name = %s, num_players = %d\n",
             game->players[game->num_players - 1].name, game->num_players);
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
    signal(SIGINT, warnhandler);

    // Check arguments
    check_args(argc, argv);

    // init queues
    new_connection_msqid =
        msgget(NEW_CONNECTION_MSGKEY, IPC_CREAT | S_IRUSR | S_IWUSR);
    game_msqid = msgget(GAME_MSGKEY, IPC_CREAT | S_IRUSR | S_IWUSR);

    // Init shared memory
    game_shmid =
        shmget(GAME_SHMKEY, sizeof(Game), IPC_CREAT | S_IRUSR | S_IWUSR);
    game = shmat(game_shmid, NULL, 0);

    // init board shared memory
    game->board_rows = atoi(argv[1]);
    game->board_cols = atoi(argv[2]);
    game->board_shmid =
        shmget(BOARD_SHMKEY, sizeof(char) * game->board_rows * game->board_cols,
               IPC_CREAT | S_IRUSR | S_IWUSR);
    board = shmat(game->board_shmid, NULL, 0);
    init_board(board);

    // init game state
    game->num_players = 0;
    game->curr_player_id = 0;
    game->players[0].token = *argv[3];
    game->players[1].token = *argv[4];
    // Set winner as null as char[32]
    memset(game->winner, 0, sizeof(char) * 32);
    game->game_over = false;

    // Connections
    accept_conn(); // Players are blocked

    info("Game Started!\n");

    int prev_player_semid = -1;
    do {
        info("Current player id: %d\n", game->curr_player_id);

        // Get current player info
        int curr_player_semid = game->players[game->curr_player_id].semid;
        char *username = game->players[game->curr_player_id].name;

        info("Unlocking player: %d\n", curr_player_semid);
        if (prev_player_semid == -1 ||
            (prev_player_semid != curr_player_semid)) {
            // Unlock other player's semaphore
            sem_release(curr_player_semid);
        }

        // Game loop
        info("waiting for msg\n");
        GameMsg msg;
        if (game_msgrcv(game_msqid, &msg) != -1) {
            info("message recieved, %lu\n", msg.mtype);
            switch (msg.mtype) {
            case Move: {
                info("On Move\n");
                GameMsgData md = msg.mdata;
                msg.mtype = insert_symbol(board, md.move.token, md.move.col);

                if (msg.mtype == GameTie) {
                    strcpy(game->winner, "\0");
                    // Set global game over
                    game->game_over = true;
                    game_msgsnd(game_msqid, &msg);
                    info("Game Tie!\n");

                    // release both clients so they can exit gracefully
                    for (int i = 0; i < 2; ++i) {
                        sem_release(game->players[i].semid);
                    }
                    break;
                }

                // Game over
                if (msg.mtype == GameOver) {
                    info("%s is the winner!\n", username);
                    // Copy user name to game->winner
                    strcpy(game->winner, username);
                    // Set global game over
                    game->game_over = true;

                    // Send game over message
                    game_msgsnd(game_msqid, &msg);
                    info("Game over!\n");

                    // release both clients so they can exit gracefully
                    for (int i = 0; i < 2; ++i) {
                        sem_release(game->players[i].semid);
                    }

                    break;
                }

                // Handle user turns
                if (msg.mtype == Continue) {
                    info("move result: %lu\n", msg.mtype);
                    game->curr_player_id = (md.player_id + 1) % 2;
                }

                game_msgsnd(game_msqid, &msg);

                prev_player_semid = curr_player_semid;
                break;
            }
            case Disconnect: {
                info("Player %lu disconnected\n", msg.mdata.player_id);
                int other_player_id = (msg.mdata.player_id + 1) % 2;
                strcpy(game->winner, game->players[other_player_id].name);
                game->game_over = true;
                game->num_players--;
                sem_release(game->players[other_player_id].semid);
                game_msgsnd(game_msqid, &msg);
                break;
            }
            default: {
                info("On default\n");
                break;
            }
            }
        }
    } while (!game->game_over);

    clear_resources();

    return 0;
}
