#include "../include/bouncy.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int toy = 0;

    if (argc > 1) {
        char *endptr;
        errno = 0;
        toy = strtol(argv[1], &endptr, 10);
        if (errno == ERANGE) {
            fprintf(stderr, "Numerical overflow.\n");
            return 1;
        }
        if (endptr == argv[1]) {
            fprintf(stderr, "Not a significant number.\n");
            return 1;
        }
    }

    if (!toy) {
        while (printf("Which of the following programs do you want to run?\n"),
               scanf("%d", &toy) != 1) {
            if (feof(stdin) || ferror(stdin)) {
                printf("Input stream error, exit.\n");
                return 1;
            }
            printf("Invalid input, please re-enter.\n");
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
        }
    }

    switch (toy) {
    case 1:
        bouncy_ball();
        break;
    default:
        printf("Invalid input!\n");
    }
    return 0;
}
