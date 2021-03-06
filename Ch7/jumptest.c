#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#define MAXLINE 4096

jmp_buf jmpbuffer;

#define TOK_ADD 5
void do_line(char *);
void cmd_add(void);
int get_token(void);

int main(void)
{
    char line[MAXLINE];
    if (setjmp(jmpbuffer) != 0)
        printf("error");
    while (fgets(line, MAXLINE, stdin) != NULL)
        do_line(line);
    exit(0);
}

char *tok_ptr;

/* global pointer for get_token() */
/* process one line of input */
void do_line(char *ptr)
{
    int cmd;
    tok_ptr = ptr;
    while ((cmd = get_token()) > 0)
    {
        switch (cmd)
        { /* one case for each command */
        case TOK_ADD:
            cmd_add();
            break;
        }
    }
}

void cmd_add(void)
{
    int token;
    token = get_token();
    if (token < 0) /* an error has occurred */
        longjmp(jmpbuffer, 1);
    /* rest of processing for this command */
}

int get_token(void)
{
    /* fetch next token from line pointed to by tok_ptr */
}