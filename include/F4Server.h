#ifndef F4Server_h
#define F4Server_h

#include <stdbool.h>

enum MoveResult { COLINVALID, COLFULL, GAMEOVER, CONTINUE };

bool outOfBounds(int i, int j, int rows, int cols);

int countInDirection(char *B, char symbol, int dx, int dy, int i, int j,
                     int rows, int cols);

bool gameOver(char *B, int i, int j, int rows, int cols);

enum MoveResult insertSymbol(char *B, char symbol, int j, int rows, int cols);

#endif
