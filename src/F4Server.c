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
        printf("'%s' is not a number\n", argv[1]);

        print_usage();
        exit(-1);
    }

    int col = atoi(argv[2]);
    if (col == 0) {
        printf("'%s' is not a number\n", argv[2]);

        print_usage();
        exit(-1);
    }

    if (row < 5 || col < 5) {
        printf("row and col must both be greater than or equal to 5\n");
        print_usage();
        exit(-1);
    }

    if (strlen(argv[3]) != 1 || strlen(argv[4]) != 1) {
        printf("player symbol must be 1 character only\n");
        print_usage();
        exit(-1);
    }
    char player1_symbol = *argv[3];
    char player2_symbol = *argv[4];

    if ((int)player1_symbol == (int)player2_symbol) {
        printf("player1_symbol must be different from player2_symbol\n");
        print_usage();
        exit(-1);
    }
}

void errExit(const char *msg) {
    perror(msg);
    cleanResources();
    exit(1);
}

void cleanResources() {
    printf("\ncleaning resources\n");
    shmdt(board);
    shmctl(board_shmid, IPC_RMID, NULL);
    shmdt(game);
    shmctl(game_shmid, IPC_RMID, NULL);
    msgctl(game_msqid, IPC_RMID, NULL);
    msgctl(new_connection_msqid, IPC_RMID, NULL);
    shmdt(client_data);
    shmctl(client_data_shmid, IPC_RMID, NULL);
}

bool outOfBounds(int i, int j, int rows, int cols) {
    return j < 0 || j > cols - 1 || i < 0 || i > rows - 1;
}

int countInDirection(char *B, char symbol, int dx, int dy, int i, int j,
                     int rows, int cols) {
    int count = 0;
    while (!outOfBounds(i, j, rows, cols) && B[(i * cols) + j] == symbol) {
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
bool gameOver(char *B, int i, int j, int rows, int cols) {
    bool won = false;
    char symbol = B[(i * cols) + j];

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            int count = countInDirection(B, symbol, dx, dy, i, j, rows, cols) +
                        countInDirection(B, symbol, -dx, -dy, i - dy, j - dx,
                                         rows, cols);
            won |= count >= 4;
            if (won) {
                goto exit;
            }
        }
    }

exit:
    return won;
}

enum GameMsgType insertSymbol(char *B, char symbol, int j, int rows, int cols) {
    // colonna non valida
    if (outOfBounds(0, j, rows, cols)) {
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

    return gameOver(B, i, j, rows, cols) ? GAMEOVER : CONTINUE;
}

void siginthandler(int code) { cleanResources(); }

void accept_connections() {
    while (game->num_players < 2) {
        game->num_players++;

        NewConnectionMsg new_connection_msg;

        printf("waiting for new connections\n");
        new_connection_msgrcv(new_connection_msqid, &new_connection_msg);
        char *name = new_connection_msg.mdata.req.name;
        printf("name = %s, num_players = %d\n", name, game->num_players);

        if (game->num_players == 1) {
            strcpy(game->players[game->num_players - 1], name);

            new_connection_msg.mtype = NewConnectionReq;
            new_connection_msg.mdata.res.game_shmid = game_shmid;
            new_connection_msg.mdata.res.game_msqid = game_msqid;
            new_connection_msg.mdata.res.player_semid = 42069;
            new_connection_msgsnd(new_connection_msqid, &new_connection_msg);
        } else {
            strcpy(game->players[game->num_players - 1], name);

            if (strcmp(game->players[game->num_players - 1],
                       game->players[game->num_players - 2]) == 0) {
                game->num_players--;

                // new_connection_set_error(&new_connection_msg);
                new_connection_msg.mtype = NameAlreadyTaken;
                new_connection_msg.mdata.res.game_msqid = -1;
                new_connection_msg.mdata.res.game_shmid = -1;
                new_connection_msg.mdata.res.player_semid = -1;
                new_connection_msgsnd(new_connection_msqid,
                                      &new_connection_msg);
            } else {
                // game init
                // release first player, suspend second player
                // new_connection_set_resources(&new_connection_msg);
                new_connection_msg.mtype = NewConnectionReq;
                new_connection_msg.mdata.res.game_shmid = game_shmid;
                new_connection_msg.mdata.res.game_msqid = game_msqid;
                new_connection_msg.mdata.res.player_semid = 42069;
                new_connection_msgsnd(new_connection_msqid,
                                      &new_connection_msg);
            }
        }
    }
}

int main(int argc, char **argv) {
    signal(SIGINT, siginthandler);

    //     ' ', ' ', ' ', ' ', ' ',
    //     ' ', ' ', 'O', 'X', ' ',
    //     'X', ' ', 'X', ' ', ' ',
    //     'X', ' ', 'O', 'O', 'O',
    //     'X', 'O', ' ', 'O', ' ',
    // };
    //
    // switch (insertSymbol(board, 'X', 1, 5, 5)) {
    // case COLINVALID: {
    //     printf("COLNOTVALID\n");
    //     break;
    // }
    // case COLFULL: {
    //     printf("COLNOTEMPTY\n");
    //     break;
    // }
    // case GAMEOVER: {
    //     printf("GAMEOVER\n");
    //     break;
    // }
    // case CONTINUE:
    //     printf("CONTINUE\n");
    //     break;
    // }

    int rows = 5;
    int cols = 5;
    // int rows = atoi(argv[1]);
    // int cols = atoi(argv[2]);

    // initialize global variables
    new_connection_msqid =
        msgget(NEW_CONNECTION_MSGKEY, IPC_CREAT | S_IRUSR | S_IWUSR);
    game_msqid = msgget(GAME_MSGKEY, IPC_CREAT | S_IRUSR | S_IWUSR);
    game_shmid =
        shmget(GAME_SHMKEY, sizeof(Game), IPC_CREAT | S_IRUSR | S_IWUSR);
    board_shmid = shmget(BOARD_SHMKEY, sizeof(char) * rows * cols,
                         IPC_CREAT | S_IRUSR | S_IWUSR);
    client_data_shmid = shmget(CLIENT_DATA_SHMKEY, sizeof(ClientData),
                               IPC_CREAT | S_IRUSR | S_IWUSR);
    game = shmat(game_shmid, NULL, 0);
    board = shmat(board_shmid, NULL, 0);
    client_data = shmat(client_data_shmid, NULL, SHM_RDONLY);

    printf("game_shmid = %d, game_msqid = %d, board_shmid = %d\n", game_shmid,
           game_msqid, board_shmid);

    // initialize game state
    game->board_shmid = board_shmid;
    game->board_rows = rows;
    game->board_cols = cols;
    game->num_players = 0;
    game->turn = 0;

    // initialize board array
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            board[(i * cols) + j] = ' ';
        }
    }

    accept_connections();

    printf("waiting for player moves\n");
    // struct semid_ds sem_players[2];
    GameMsg msg;
    while (game_msgrcv(game_msqid, &msg) != -1) {
        switch (msg.mtype) {
        case MOVE: {
        }
        case DISCONNECT: {
            // accept_connections();
        }
        default: {
            break;
        }
        }
    }
}
