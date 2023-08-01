#include "F4lib.h"

void semOp(int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1)
        errExit("semop failed");
}

int new_connection_msgsnd(int new_connection_msqid, NewConnectionMsg *msg) {
    int ret = msgsnd(new_connection_msqid, msg,
                     sizeof(NewConnectionMsg) - sizeof(long), 0);

    if (ret == -1) {
        errExit("new_connection_msgsnd");
    }

    return ret;
}

int new_connection_msgrcv(int new_connection_msqid, NewConnectionMsg *msg) {
    int ret = msgrcv(new_connection_msqid, msg,
                     sizeof(NewConnectionMsg) - sizeof(long), 0, 0);
    if (ret == -1) {
        errExit("new_connection_msgrcv");
    }

    return ret;
}

int game_msgsnd(int game_msqid, GameMsg *msg) {
    int ret = msgsnd(game_msqid, msg, sizeof(GameMsg) - sizeof(long), 0);

    if (ret == -1) {
        errExit("game_msgsnd");
    }

    return ret;
}

int game_msgrcv(int game_msqid, GameMsg *msg) {
    int ret = msgrcv(game_msqid, msg, sizeof(GameMsg) - sizeof(long), 0, 0);

    if (ret == -1) {
        errExit("game_msgrcv");
    }

    return ret;
}
