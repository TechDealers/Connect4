#include <stdio.h>

void drawBoard(char **B, int rows, int cols);

int main(int argc, char **argv) {}

// TODO: Forse sarebbe meglio passare il memid al posto
// della matrice?
void drawBoard(char **B, int rows, int cols) {
    // top border
    printf("┌─");
    for (int i = 0; i < cols - 1; ++i) {
        printf("──┬─");
    }
    printf("──┐\n");

    for (int i = 0; i < rows; ++i) {
        // player symbols
        for (int j = 0; j < cols; ++j) {
            printf("│ %c ", B[i][j]);
        }
        printf("│\n");

        // middle borders
        if (i < rows - 1) {
            printf("├─");
            for (int j = 0; j < cols - 1; ++j) {
                printf("──┼─");
            }
            printf("──┤\n");
        }
    }

    // bottom border
    printf("└─");
    for (int i = 0; i < cols - 1; ++i) {
        printf("──┴─");
    }
    printf("──┘\n");
}
