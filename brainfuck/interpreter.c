/* Type your code here, or load an example. */
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

char read_input() {
    char c;
    int chars = fread(&c,1,1,stdin);
    if (chars != 1) return '\0';
    return c;
}

void left_bracket(char *program, char *memory, char **ip_ptr, char *data) {
    if (*data == 0) {
        int count = 0;
        while (*ip_ptr < program + 1000) {
            if (**ip_ptr == '[') ++count;
            else if (**ip_ptr == ']') --count;
            if (count == 0) break;
            ++(*ip_ptr);
        }
        if (count != 0) {
            fprintf(stderr, "Unmatched brackets.\n");
            exit(1);
        }
    }
}

void right_bracket(char *program, char *memory, char **ip_ptr, char *data) {
    if (*data != 0) {
        int count = 0;
        while (*ip_ptr >= program) {
            if (**ip_ptr == ']') ++count;
            else if (**ip_ptr == '[') --count;
            if (count == 0) break;
            --(*ip_ptr);
        }
        if (count != 0) {
            fprintf(stderr, "Unmatched brackets.\n");
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./interpreter <program> <input >output \n");
        exit(1);
    }

    int prog = open(argv[1], O_RDONLY);
    if (prog < 0) {
        perror("Couldn't open prog for reading");
        exit(1);
    }

    char *program = calloc(1000,sizeof(char));
    int numchars = read(prog, program, 1000);
    if (numchars < 0) {
        perror("Couldn't read file for reading");
        exit(1);
    }

    char *memory = calloc(1000,sizeof(char));

    char *IP = program;
    char *data = memory;

    bool running = true;
    while (running) {
        switch (*IP) {
            case '[': left_bracket(program, memory, &IP, data);
            break;
            case ']': right_bracket(program, memory, &IP, data);
            break;
            case '<': --data;
                if (data == memory - 1) data += 1000;
            break;
            case '>': ++data;
                if (data == memory + 1000) data -= 1000;
            break;
            case '+': ++(*data);
            break;
            case '-': --(*data);
            break;
            // case '.': printf("Memory at %d: %d\n", (int) (data - memory), (int)(*data));
            case '.': printf("%c", *data);
            break;
            case ',': *data = read_input();
            break;
            case 0: running = false;
            break;
        }
        ++IP;
    }

    close(prog);
    free(program);
    free(memory);
    return 0;
}
