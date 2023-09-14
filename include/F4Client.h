/************************************
* VR471635 - VR472194 - VR497290
* Michael Cisse - Abel Hristodor - Safouane Ben Baa
* 14/09/2023
*************************************/

#ifndef F4Client_H
#define F4Client_H

#include "F4lib.h"

Resources resources;
Player player;

void draw_board(char *B, int rows, int cols);
void siginthandler(int code);
void sigusr1handler(int code);
int get_random_move(int MIN, int MAX);

#endif
