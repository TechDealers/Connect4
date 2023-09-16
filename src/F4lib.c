/************************************
 * VR471635 - VR472194 - VR497290
 * Michael Cisse - Abel Hristodor - Safouane Ben Baa
 * 14/09/2023
 *************************************/

#include "F4lib.h"
#include <stdio.h>

void sem_op(int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1)
        err_exit("semop failed");
}

void info(const char *format, ...) {
    va_list args;
    va_start(args, format);

    printf("[INFO] ");
    vprintf(format, args);

    va_end(args);
}

int new_connection_msgsnd(int new_connection_msqid, NewConnectionMsg *msg) {
    int ret;
    do {
        ret = msgsnd(new_connection_msqid, msg,
                     sizeof(NewConnectionMsg) - sizeof(long), 0);
    } while (errno == EINTR);

    if (ret == -1) {
        err_exit("new_connection_msgsnd");
    }

    return ret;
}

int new_connection_msgrcv(int new_connection_msqid, NewConnectionMsg *msg) {
    int ret;
    do {
        ret = msgrcv(new_connection_msqid, msg,
                     sizeof(NewConnectionMsg) - sizeof(long), 0, 0);
    } while (errno == EINTR);

    if (ret == -1) {
        err_exit("new_connection_msgrcv");
    }

    return ret;
}

int game_msgsnd(int game_msqid, GameMsg *msg) {
    int ret;
    do {
        ret = msgsnd(game_msqid, msg, sizeof(GameMsg) - sizeof(long), 0);
    } while (errno == EINTR);

    if (ret == -1) {
        err_exit("game_msgsnd");
    }

    return ret;
}

int game_msgrcv(int game_msqid, GameMsg *msg) {
    int ret;
    do {
        ret = msgrcv(game_msqid, msg, sizeof(GameMsg) - sizeof(long), 0, 0);
    } while (errno == EINTR);

    if (ret == -1) {
        err_exit("game_msgrcv");
    }

    return ret;
}
