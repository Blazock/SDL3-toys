#include "../include/bouncy.h"
#include <stdio.h>

int main() {
    printf("Which of the following programs do you want to run?\n");
    int toy;
    scanf("%d", &toy);
    switch (toy) {
    case 1:
        bouncy_ball();
        break;
    default:
        printf("Invalid input!");
    }
    return 0;
}
