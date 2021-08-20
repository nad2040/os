#include <stdlib.h>
#include <stdio.h>

void exitHandler1() {
    printf("exitHandler1\n");
}
void exitHandler2() {
    printf("exitHandler2\n");
}
void exitHandler3() {
    printf("exitHandler3\n");
}

int main() {
    if(atexit(exitHandler1)) exit(1);
    if(atexit(exitHandler2)) exit(1);
    if(atexit(exitHandler3)) exit(1);
    if(atexit(exitHandler1)) exit(1);
    printf("main start\n");
    for(int i=0; i<5; ++i) {
        printf("hello\n");
        //if (i==3) return i;
    }
    //return 2;
    printf("main done\n");
}