/************************************
* VR471635 - VR472194 - VR497290
* Michael Cisse - Abel Hristodor - Safouane Ben Baa
* 14/09/2023
*************************************/

#ifndef F4Server_h
#define F4Server_h

#include "F4lib.h"

#define GAME_SHMKEY ftok("src/F4Server.c", getpid() + 1)
#define GAME_MSGKEY ftok("src/F4Server.c", getpid() + 2)
#define BOARD_SHMKEY ftok("src/F4Server.c", getpid() + 3)
#define CLIENT_DATA_SHMKEY ftok("src/F4Server.c", getpid() + 4)

// necessary global variables
char *board;
int game_shmid;
Game *game;
int new_connection_msqid;
int game_msqid;
int server_semid;
int computer_pid;

bool out_of_bounds(int i, int j);

int count_in_direction(char *B, char symbol, int dx, int dy, int i, int j);

bool game_over(char *B, int i, int j);
bool game_tie(char *B);
void accept_conn(int server_semid);

enum GameMsgType insert_symbol(char *B, char symbol, int j);

union semun {
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO
                              (Linux-specific) */
};

#endif
