#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>

#include "shell.h"
extern shell_state_t shell_state;

int
cd(int argc, char **argv) // called without fork, so return
{
	if (argc > 2) {
		fprintf(stderr, "cd: too many arguments\n");
		return EXIT_FAILURE;
	}

	char *path;
	if (argc == 1) {
		path = getenv("HOME");
		if (path == NULL) {
			uid_t uid = getuid();
			struct passwd *passwd = getpwuid(uid);
			path = passwd->pw_dir;
		}
	}
	else { // argc == 2
		path = argv[1];
	}

	if (chdir(path) == -1) {
		fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void
echo(int argc, char **argv) // called with fork, so exit
{
	char *arg;
	for (int i = 1; i < argc; ++i) {
		arg = argv[i];
		if (strcmp(arg, "$$") == 0) {
			printf("%d", shell_state.current_pid);
		}
		else if (strcmp(arg, "$?") == 0) {
			printf("%d", shell_state.last_exit_status);
		}
		else {
			printf("%s", arg);
		}

		if (i < argc - 1) {
			printf(" ");
		}
		else {
			printf("\n");
		}
	}

	exit(EXIT_SUCCESS);
}
