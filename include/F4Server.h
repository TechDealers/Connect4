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

bool outOfBounds(int i, int j, int rows, int cols);

int countInDirection(char *B, char symbol, int dx, int dy, int i, int j,
                     int rows, int cols);

bool gameOver(char *B, int i, int j, int rows, int cols);

enum GameMsgType insertSymbol(char *B, char symbol, int j, int rows, int cols);

void cleanResources();

#endif
