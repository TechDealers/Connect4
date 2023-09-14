/************************************
* VR471635 - VR472194 - VR497290
* Michael Cisse - Abel Hristodor - Safouane Ben Baa
* 14/09/2023
*************************************/

#ifndef F4lib_h
#define F4lib_h
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/sem.h>

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

void sem_wait(int semid, int val);
void sem_release(int semid, int val);

void clear_resources();

#define NEW_CONNECTION_MSGKEY ftok("src/F4Server.c", 1)

#define STRSIZE 32

typedef struct {
    char name[STRSIZE];
    int semid;
    int pid;
    char token;
} Player;

typedef struct {
    Player players[2];

    int curr_player_id;
    int num_players;
    int board_shmid;
    int board_rows;
    int board_cols;
    bool game_over;
    char winner[STRSIZE];
} Game;

typedef struct {
    int game_shmid;
    int game_msqid;
    int player_id;
    int timeout;
} Resources;

enum NewConnectionMsgType { NewConnection = 1, NameAlreadyTaken = 2 };
typedef struct {
    long mtype;

    union {
        struct {
            char name[STRSIZE];
            int pid;
            bool computer;
        } req;

        Resources res;
    } mdata;
} NewConnectionMsg;
int new_connection_msgrcv(int new_connection_msqid, NewConnectionMsg *msg);
int new_connection_msgsnd(int new_connection_msqid, NewConnectionMsg *msg);

enum GameMsgType {
    Nil = 1,
    Move,
    Disconnect,
    ColInvalid,
    ColFull,
    GameOver,
    GameTie,
    Continue,
};

// typedef enum GameMsgType MsgKind;
//
// typedef struct {
//     long mtype;
//
//     union {
//         union {
//             struct {
//                 char name[STRSIZE];
//             } req;
//
//             Resources res;
//         } new_connection;
//
//         union {
//             struct {
//                 int player_id;
//             } req;
//
//             void *res;
//         } disconnect;
//
//         union {
//             struct {
//                 int player_id;
//                 int col;
//             } req;
//
//             MsgKind res;
//         } move;
//     } mdata;
// } Msg;

typedef struct {
    int player_id;
    struct {
        int col;    // -1
        char token; // '\0'
    } move;
} GameMsgData;

typedef struct {
    long mtype; // One of enum GameMsgType

    // Game msg struct
    GameMsgData mdata;
} GameMsg;

int game_msgsnd(int game_msqid, GameMsg *msg);
int game_msgrcv(int game_msqid, GameMsg *msg);

#endif
