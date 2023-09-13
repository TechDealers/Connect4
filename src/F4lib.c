#include "F4lib.h"

void sem_op(int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1)
        err_exit("semop failed");
}

void sem_wait(int semid) {
    // Lock the semaphore - decrement it by 1
    sem_op(semid, 0, -1);
}

void info(const char *format, ...) {
    va_list args;
    va_start(args, format);

    printf("[INFO] ");
    vprintf(format, args);

    va_end(args);
}

void sem_release(int semid) {
    // Unlock the semaphore - increment it by 1
    sem_op(semid, 0, 1);
}

int new_connection_msgsnd(int new_connection_msqid, NewConnectionMsg *msg) {
    int ret = msgsnd(new_connection_msqid, msg,
                     sizeof(NewConnectionMsg) - sizeof(long), 0);

    if (ret == -1) {
        err_exit("new_connection_msgsnd");
    }

    return ret;
}

int new_connection_msgrcv(int new_connection_msqid, NewConnectionMsg *msg) {
    int ret = msgrcv(new_connection_msqid, msg,
                     sizeof(NewConnectionMsg) - sizeof(long), 0, 0);
    if (ret == -1) {
        err_exit("new_connection_msgrcv");
    }

    return ret;
}

int game_msgsnd(int game_msqid, GameMsg *msg) {
    int ret = msgsnd(game_msqid, msg, sizeof(GameMsg) - sizeof(long), 0);

    if (ret == -1) {
        err_exit("game_msgsnd");
    }

    return ret;
}

int game_msgrcv(int game_msqid, GameMsg *msg) {
    int ret = msgrcv(game_msqid, msg, sizeof(GameMsg) - sizeof(long), 0, 0);

    if (ret == -1) {
        err_exit("game_msgrcv");
    }

    return ret;
}
