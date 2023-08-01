#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "F4Client.h"
#include "F4lib.h"
#include "unistd.h"

void errExit(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char **argv) {
    int new_connection_msqid =
        msgget(NEW_CONNECTION_MSGKEY, IPC_CREAT | S_IRUSR | S_IWUSR);
    NewConnectionMsg new_connection_msg;
    bool invalid = false;
    do {
        char name[STRSIZE];
        printf("insert your username(MAX 32 characters): ");
        scanf("%s", name);

        new_connection_msg.mtype = NewConnectionReq;
        strcpy(new_connection_msg.mdata.req.name, name);
        new_connection_msgsnd(new_connection_msqid, &new_connection_msg);

        new_connection_msgrcv(new_connection_msqid, &new_connection_msg);
        invalid = new_connection_msg.mtype == NameAlreadyTaken;
        if (invalid) {
            printf("username already taken\n");
        }
    } while (invalid);

    Resources resources = new_connection_msg.mdata.res;
    Game *game;
    char *board;
    ClientData *client_data;

    game = shmat(resources.game_shmid, NULL, SHM_RDONLY);
    board = shmat(game->board_shmid, NULL, SHM_RDONLY);
    client_data = shmat(game->client_data_shmid, NULL, 0);

    drawBoard(board, game->board_rows, game->board_cols);
}

// do {
//     // P(client)
//
//     // send move to server
//
//     // recv response from server
// } while (/* not gameover */ 1);

// handle exit

// TODO: Forse sarebbe meglio passare il memid al posto
// della matrice?
void drawBoard(char *B, int rows, int cols) {
    // top border
    printf("┌─");
    for (int i = 0; i < cols - 1; ++i) {
        printf("──┬─");
    }
    printf("──┐\n");

    for (int i = 0; i < rows; ++i) {
        // player symbols
        for (int j = 0; j < cols; ++j) {
            printf("│ %c ", B[(i * cols) + j]);
        }
        printf("│\n");

        // middle borders
        if (i < rows - 1) {
            printf("├─");
            for (int j = 0; j < cols - 1; ++j) {
                printf("──┼─");
            }
            printf("──┤\n");
        }
    }

    // bottom border
    printf("└─");
    for (int i = 0; i < cols - 1; ++i) {
        printf("──┴─");
    }
    printf("──┘\n");
}
