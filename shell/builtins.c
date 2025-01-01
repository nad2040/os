#include "builtins.h"

#include <sys/types.h>

#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shell.h"

int
cd(int argc, char **argv)
{
	char *path;

	if (argc > 2) {
		warnx("cd: too many arguments");
		return EXIT_FAILURE;
	}

	if (argc == 1) {
		path = getenv("HOME");
		if (path == NULL) {
			uid_t uid = getuid();
			struct passwd *passwd = getpwuid(uid);
			if (passwd == NULL) {
				warn("unable to retrieve user info");
			}
			path = passwd->pw_dir;
		}
	} else {
		path = argv[1];
	}

	if (chdir(path) == -1) {
		warn("cd: %s", path);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void
echo(int argc, char **argv)
{
	char *arg;
	char *arg_env_value;

	for (int i = 1; i < argc; ++i) {
		bool skip_space = false;

		arg = argv[i];
		if (strcmp(arg, "$$") == 0) {
			(void)printf("%d", shell_state.current_pid);
		} else if (strcmp(arg, "$?") == 0) {
			(void)printf("%d", shell_state.last_exit_status);
		} else if (strcmp(arg, "$SHELL") == 0) {
			(void)printf("%s", shell_state.full_path);
		} else if (strncmp(arg, "$", 1) == 0) {
			if (strcmp(arg + 1, "") == 0) {
				(void)printf("$");
			} else {
				arg_env_value = getenv(arg + 1);
				if (arg_env_value == NULL) {
					skip_space = true;
				} else {
					(void)printf("%s", arg_env_value);
				}
			}
		} else {
			(void)printf("%s", arg);
		}

		if (i < argc - 1) {
			if (!skip_space) {
				(void)printf(" ");
			}
		} else {
			(void)printf("\n");
		}
	}

	exit(EXIT_SUCCESS);
}
