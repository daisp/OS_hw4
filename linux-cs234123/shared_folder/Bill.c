//
// Created by dais on 6/20/18.
//
#include <stdio.h>

int main(void) {
	int i=0;
    while (++i != 120) {
        sleep(1);
        printf("*");
        fflush(stdout);
    }
    return 0;
}
