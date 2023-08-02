#ifndef F4Server_h
#define F4Server_h

#include "F4lib.h"
#include <stdbool.h>

#define GAME_SHMKEY ftok("src/F4Server.c", getpid() + 1)
#define GAME_MSGKEY ftok("src/F4Server.c", getpid() + 2)
#define BOARD_SHMKEY ftok("src/F4Server.c", getpid() + 3)
#define CLIENT_DATA_SHMKEY ftok("src/F4Server.c", getpid() + 4)

// necessary global variables
int board_shmid;
char *board;
int game_shmid;
Game *game;
int client_data_shmid;
ClientData *client_data;
int new_connection_msqid;
int game_msqid;

bool out_of_bounds(int i, int j);

int count_in_direction(char *B, char symbol, int dx, int dy, int i, int j);

bool game_over(char *B, int i, int j);

enum GameMsgType insert_symbol(char *B, char symbol, int j);

void clear_resources();

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

#endif
