//
//  F4lib.h
//  
//
//  Created by Safouane Ben Baa on 25/04/2023.
//

#ifndef F4lib_h
#define F4lib_h
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>

#define SEMKEY 48
#define MEMKEY 62

#define BREAKPOINT printf("\nLine Number %s->%s:%d\n\n", __FILE__, __FUNCTION__, __LINE__)

/* errsemOpExit is a support function to manipulate a semaphore's value
 * of a semaphore set. semid is a semaphore set identifier, sem_num is the
 * index of a semaphore in the set, sem_op is the operation performed on sem_num
 */
void semOp (int semid, unsigned short sem_num, short sem_op);
void errExit(const char *msg);

void errExit(const char *msg) {
    perror(msg);
    exit(1);
}
void semOp (int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};
    
    if (semop(semid, &sop, 1) == -1)
        errExit("semop failed");
}

#endif