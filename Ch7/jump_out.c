#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

jmp_buf jmpbuffer;

void func4() {
    longjmp(jmpbuffer, 1);
}
void func3() {
    func4();
}
void func2() {
    func3();
}
void func1() {
    func2();
}
void func0() {
    func1();
}

int main() {
    char line[4096];

    if (setjmp(jmpbuffer) != 0)
        printf("error\n");

    while (fgets(line, 4096, stdin) != NULL) {
        fputs(line, stdout);
        func0();
    }

    printf("exiting main");
    return 0;
}