#include "F4lib.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <errno.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/stat.h>

void print_usage() {
    printf("Usage: ./F4Server {rows} {cols} {player1_symbol} {player2_symbol}\n");
}
void check_args(int argc, char* argv[]){
    if(argc != 5) {
        print_usage();
        exit(-1);
    }

    int row = atoi(argv[1]);
    if (row == 0) {
        printf("'%s' is not a number\n", argv[1]);

        print_usage();
        exit(-1);
    }


    int col = atoi(argv[2]);
    if (col == 0) {
        printf("'%s' is not a number\n", argv[2]);

        print_usage();
        exit(-1);
    }

    if (row < 5 || col < 5) {
        printf("row and col must both be greater than or equal to 5\n");
        print_usage();
        exit(-1);
    }

    if (strlen(argv[3]) != 1 || strlen(argv[4]) != 1) {
        printf("player symbol must be 1 character only\n");
        print_usage();
        exit(-1);
    }
    char player1_symbol = *argv[3];
    char player2_symbol = *argv[4];

    if ((int)player1_symbol == (int)player2_symbol) {
        printf("player1_symbol must be different from player2_symbol\n");
        print_usage();
        exit(-1);
    }

}
int main(int argc, char **argv) {
    //controllo argomenti
    check_args(argc, argv);
}