#include <sys/wait.h>
#include "ch8.h"

int main(void)
{
    int status;
    if ((status = system("date")) < 0)
        { fprintf(stderr, "system() error"); exit(1); }
    pr_exit(status);
    if ((status = system("nosuchcommand")) < 0)
        { fprintf(stderr, "system() error"); exit(1); }
    pr_exit(status);
    if ((status = system("who; exit 44")) < 0)
        { fprintf(stderr, "system() error"); exit(1); }
    pr_exit(status);
    exit(0);
}