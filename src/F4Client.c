#include <stdio.h>

void drawBoard(char **B, int rows, int cols) {
    // top border
    printf("┌");
    for (int i = 0; i < cols - 1; ++i) {
        printf("─┬");
    }
    printf("─┐\n");

    // player symbols
    for (int i = 0; i < rows; ++i) {
        // left border
        printf("│");
        for (int j = 0; j < cols; ++j) {
            printf("%c│", B[i][j]);
        }

        if (i < rows - 1) {
            printf("\n├");
            for (int j = 0; j < cols - 1; ++j) {
                printf("─┼");
            }
            // right border
            printf("─┤");
        }
        printf("\n");
    }

    // bottom border
    printf("└");
    for (int i = 0; i < cols - 1; ++i) {
        printf("─┴");
    }
    printf("─┘\n");
}

int main(int argc, char **argv) {}
