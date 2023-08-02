//
//  F4lib.h
//
//
//  Created by Safouane Ben Baa on 25/04/2023.
//

#ifndef F4lib_h
#define F4lib_h
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <stdarg.h>
#include <stdio.h>

#define MAX_PLAYERS 2
#define BREAKPOINT                                                             \
    printf("\nLine Number %s->%s:%d\n\n", __FILE__, __FUNCTION__, __LINE__)

/* errsemOpExit is a support function to manipulate a semaphore's value
 * of a semaphore set. semid is a semaphore set identifier, sem_num is the
 * index of a semaphore in the set, sem_op is the operation performed on sem_num
 */
void info(const char *format, ...);
void sem_op(int semid, unsigned short sem_num, short sem_op);
void err_exit(const char *msg);

void sem_wait(int semid);
void sem_release(int semid);

#define NEW_CONNECTION_MSGKEY ftok("src/F4Server.c", 1)

#define STRSIZE 32

typedef struct {
    char name[STRSIZE];
    int player_semid;
    char token;
} Player;

typedef struct {
    Player players[2];
    char tokens[2];

    int num_players;
    int board_shmid;
    int board_rows;
    int board_cols;
    int client_data_shmid;
    bool game_over;
    char winner[STRSIZE];
    int turn;
} Game;

typedef struct {
    int game_shmid;
    int game_msqid;
    int player_semid;
} Resources;

enum NewConnectionMsgType { NewConnectionReq = 1, NameAlreadyTaken = 2 };
typedef struct {
    long mtype;

    union {
        struct {
            char name[STRSIZE];
        } req;

        Resources res;
    } mdata;
} NewConnectionMsg;
int new_connection_msgrcv(int new_connection_msqid, NewConnectionMsg *msg);
int new_connection_msgsnd(int new_connection_msqid, NewConnectionMsg *msg);

enum GameMsgType {
    NIL = 1,
    MOVE,
    DISCONNECT,
    COLINVALID,
    COLFULL,
    GAMEOVER,
    CONTINUE,
};

typedef struct {
    int player_semid;
    struct {
        int col; // -1
        char token; // ''
    } move;
} MoveData;

typedef struct {
    long mtype; // One of enum GameMsgType

    // Game msg struct
    MoveData mdata;
} GameMsg;

int game_msgsnd(int game_msqid, GameMsg *msg);
int game_msgrcv(int game_msqid, GameMsg *msg);


typedef struct {
    int id;
    int move;
} ClientData;

#endif
