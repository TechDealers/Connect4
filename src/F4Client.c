#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "F4Client.h"
#include "F4lib.h"

void err_exit(const char *msg) {
    perror(msg);
    exit(1);
}

void sigusrhandler(int code) {
    printf("\nGame is shutting down!\n");
    exit(0);
}

void siginthandler(int code) {
    info("\nUser abandoned the game!\n");
    GameMsg msg = {
        .mtype = Disconnect,
        .mdata = {
            .player_id = resources.player_id,
            .move = {
                .col = -1,
                .token = '\0'
            }
        }
    };
    
    game_msgsnd(resources.game_msqid, &msg);
    exit(0);
}

int get_random_move(int MIN, int MAX) {
    return (rand() % (MAX - MIN + 1)) + MIN;
}

int main(int argc, char **argv) {
    bool computer = false;

    int pid = getpid();
    
    if (argc == 3) {
        if(*argv[2] == '*') {
            computer = true;
        }
    }
    
    if (computer) {
        pid = fork();
        if (pid == 0) {
            close(STDOUT_FILENO);
            // if(open("computer_stdout.txt", O_CREAT | O_TRUNC | O_APPEND | O_WRONLY, 00777) == -1) exit(-1);
            // info("TESTING");
            
            srand(getpid() + time(NULL));
        }
    }

    signal(SIGUSR1, sigusrhandler);
    signal(SIGINT, siginthandler);

    // Get new connection queue
    int new_connection_msqid =
        msgget(NEW_CONNECTION_MSGKEY, IPC_CREAT | S_IRUSR | S_IWUSR);

    // Create new connection obj
    NewConnectionMsg new_connection_msg;
    bool invalid = false; // is username valid?
    char name[STRSIZE];
    if (pid == 0){
        strcpy(name, "Computer");
    } else {
        strcpy(name, argv[1]);
    }
    
    do {

        new_connection_msg.mtype = NewConnection;
        // Copy name to msg
        strcpy(new_connection_msg.mdata.req.name, name);
        // send pid to server
        new_connection_msg.mdata.req.pid = getpid();
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
    resources = new_connection_msg.mdata.res;

    // init shared memory
    Game *game = shmat(resources.game_shmid, NULL, SHM_RDONLY);
    char *board = shmat(game->board_shmid, NULL, SHM_RDONLY);

    // init client data
    Player data = game->players[resources.player_id];
    strcpy(data.name, name);

    while (true) {
        // Block semaphore
        info("Waiting for the next player...\n");
        sem_wait(data.semid);

        draw_board(board, game->board_rows, game->board_cols);

        if (game->game_over) {
            if (game->num_players == 1) {
                info("Player Disconnected!\n");
                info("YOU WIN!\n");
            } else if (strcmp(game->winner, "\0") == 0) {
                info("DRAW\n");
            } else if (strcmp(data.name, game->winner) == 0) {
                info("YOU WIN!\n");
            } else {
                info("YOU LOSE!\n");
            }
            info("#######################\n");
            info("GAMEOVER\n");
            break;
        }

        GameMsg msg;
        do {
            int col;
            
            if (pid == 0){
                col = get_random_move(0, game->board_cols - 1);
            } else {
                // Ask user for input
                info("Insert column: ");
                scanf("%d", &col);
            }

            // Send move to server
            msg.mtype = Move;
            msg.mdata.move.col = col;
            msg.mdata.move.token = data.token;
            msg.mdata.player_id = data.semid;

            // send move
            if (game->num_players < 2) {
                info("Move ignored, not enough players.\n");
                break;
            } 
            game_msgsnd(resources.game_msqid, &msg);
            // wait for server response
            game_msgrcv(resources.game_msqid, &msg);


   
        } while (msg.mtype == ColFull || msg.mtype == ColInvalid);
        
        draw_board(board, game->board_rows, game->board_cols);
    }
}

void draw_board(char *B, int rows, int cols) {
    // Clear screen
    printf("\033[2J\033[H");

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

