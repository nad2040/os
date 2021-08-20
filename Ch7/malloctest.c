#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int i = 10;
int a[3] = {0 ,1, 2};
int* aptr;
int buninit;

void f() {
    int locali = 5;
    char* buf = "hello";
    printf("locali %p, buf %p\n", &locali, buf);
}
int main() {
    int n = 1;
    // little endian if true
     (*(char *)&n == 1) ? printf("little %p\n", &n) : printf("big %p\n", &n);
    // return 0;
    aptr = malloc(sizeof(int));
    f();
    printf("i %p, a %p, aptr %p, buninit %p\n", &i, a, &aptr, &buninit);
    //sleep(30);
    return 0;
}