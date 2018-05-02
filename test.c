#include "wthreads.h"
#include <stdlib.h>
#include <stdio.h>

char compute_cube(char* str) {
    printf("%s", str);
    return str[2];
}

int main() {
    int index1 = launch_wthread(compute_cube, "Hello ");
    int index2 = launch_wthread(compute_cube, "World!");
    waitall_wthreads();
    char c1 = wthread_return(index1);
    char c2 = wthread_return(index2);
    printf("%c %c", c1, c2);
    return 0;
}