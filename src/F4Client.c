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

void err_exit(const char *msg) {
    perror(msg);
    exit(1);
}


int main(int argc, char **argv) {
    // Create new connection obj
    NewConnectionMsg new_connection_msg;

    // Get new connection queue
    int new_connection_msqid = msgget(NEW_CONNECTION_MSGKEY, IPC_CREAT | S_IRUSR | S_IWUSR);

    bool invalid = false; // is username valid?
    do {
        char name[STRSIZE];
        printf("Insert your username (MAX 32 characters): ");
        scanf("%s", name);

        new_connection_msg.mtype = NewConnectionReq;
        // Copy name to msg
        strcpy(new_connection_msg.mdata.req.name, name);
        // Send msg
        new_connection_msgsnd(new_connection_msqid, &new_connection_msg);
        // Receive confirmation or error
        new_connection_msgrcv(new_connection_msqid, &new_connection_msg);

        invalid = new_connection_msg.mtype == NameAlreadyTaken;
        if (invalid) {
            info("username already taken\n");
        }
    } while (invalid);

    // Get game resources
    Resources resources = new_connection_msg.mdata.res;
    Game *game;
    char *board;
    ClientData *client_data;

    // Init shared memory
    game = shmat(resources.game_shmid, NULL, SHM_RDONLY);
    board = shmat(game->board_shmid, NULL, SHM_RDONLY);
    client_data = shmat(game->client_data_shmid, NULL, 0);

    do {
        info("Waiting for the next player to make a move...\n");
        // Block semaphore
        sem_wait(resources.player_semid);

        info("Your turn!\n");

        // Draw a board
        draw_board(board, game->board_rows, game->board_cols);

        GameMsg msg;
        do {
            int col;
            // Ask user for input
            info("Insert column: ");
            scanf("%d", &col);

            // Send move to server
            msg.mtype = MOVE;
            msg.mdata.move.col = col;
            msg.mdata.move.token = game->players[game->turn].token;
            msg.mdata.player_semid = resources.player_semid;

            if (game_msgsnd(resources.game_msqid, &msg) != -1) {
                if (game_msgrcv(resources.game_msqid, &msg) != -1){
                    switch (msg.mtype) {
                        case CONTINUE:
                            break;
                        
                        case GAMEOVER:
                            if (msg.mdata.player_semid == resources.player_semid) {
                                info("YOU WIN!\n");
                            } else {
                                info("YOU LOSE!\n");
                            }
                            info("#######################\n");
                            info("GAMEOVER\n");
                        
                        default:
                            break;
                        
                    }
                }
            }
        } while(msg.mtype == COLFULL || msg.mtype == COLINVALID);
        draw_board(board, game->board_rows, game->board_cols);
    } while(1);
}

void draw_board(char *B, int rows, int cols) {
    // Clear screen
    info("\033[2J\033[H");

    // top border
    info("┌─");
    for (int i = 0; i < cols - 1; ++i) {
        info("──┬─");
    }
    info("──┐\n");

    for (int i = 0; i < rows; ++i) {
        // player symbols
        for (int j = 0; j < cols; ++j) {
            info("│ %c ", B[(i * cols) + j]);
        }
        info("│\n");

        // middle borders
        if (i < rows - 1) {
            info("├─");
            for (int j = 0; j < cols - 1; ++j) {
                info("──┼─");
            }
            info("──┤\n");
        }
    }

    // bottom border
    info("└─");
    for (int i = 0; i < cols - 1; ++i) {
        info("──┴─");
    }
    info("──┘\n");
}
