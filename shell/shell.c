#include "shell.h"

#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>

#include "util.h"

void sighandler(int);

void argparse(int, char **, bool *, bool *, char **);

void loop(void);

int main(int, char **);

shell_state_t shell_state = {0};

volatile sig_atomic_t interrupted = false;

/*
 * signal handler to handle interrupts from keyboard
 */
void
sighandler(int signum)
{
	if (signum == SIGINT || signum == SIGTSTP || signum == SIGQUIT) {
		interrupted = true;
	}
}

/*
 * argparse parses the arguments from argv.
 */
void
argparse(int argc, char **argv, bool *x_flag, bool *c_flag,
         char **c_flag_script)
{
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "xc:")) != -1) {
		switch (c) {
		case 'x':
			*x_flag = true;
			break;
		case 'c':
			*c_flag = true;
			*c_flag_script = optarg;
			break;
		case '?':
			if (optopt == 'c') {
				warnx("option -%c requires an argument.",
				      optopt);
			} else if (isprint(optopt)) {
				warnx("unknown option `-%c'", optopt);
			} else {
				warnx("unknown option `\\x%x'", optopt);
			}
			/* FALLTHROUGH */
		default:
			exit(EXIT_FAILURE);
		}
	}
}

/*
 * This is the sish repl. It gets input line by line.
 * It also checks for background jobs that have completed before every reprompt.
 */
void
loop(void)
{
	size_t len;
	char *line, *stripped_line;
	asynclist_t *asynclist;

	(void)printf(PROMPT " ");

	for (;;) {
		int nread;

		line = NULL;
		nread = getline(&line, &len, stdin);
		if (nread == -1) {
			if (interrupted) {
				(void)printf("\n");
				interrupted = false;
				goto reprompt;
			}
			break;
		}
		stripped_line = strip_copy(line);
		free(line);
		if (strcmp(stripped_line, "") == 0) {
			free(stripped_line);
			goto reprompt;
		}
		asynclist = parse_asynclist(stripped_line);
		free(stripped_line);
		if (asynclist != NULL) {
			run_asynclist(asynclist, shell_state.display_cmds);
		}
	reprompt:
		check_jobs(&shell_state.background_jobs, true);
		(void)printf(PROMPT " ");
	}
	/* NOTREACHED */
}

/*
 * This is the main function for sish.
 */
int
main(int argc, char **argv)
{
	bool x_flag = false;
	bool c_flag = false;
	char *c_flag_script = NULL;
	char *full_path = NULL;

	setprogname(argv[0]);
	if ((full_path = realpath(argv[0], NULL)) == NULL) {
		err(EXIT_FAILURE, "failed to get full path of argv");
	}

	argparse(argc, argv, &x_flag, &c_flag, &c_flag_script);

	shell_state = (shell_state_t){.display_cmds = x_flag,
	                              .last_exit_status = 0,
	                              .current_pid = getpid(),
	                              .full_path = full_path,
	                              .background_jobs = {0}};

	struct sigaction action = {.sa_handler = sighandler};
	if (sigaction(SIGINT, &action, NULL) == -1 ||
	    sigaction(SIGQUIT, &action, NULL) == -1 ||
	    sigaction(SIGTSTP, &action, NULL) == -1) {
		err(EXIT_FAILURE, "failed to register signal handlers");
	}

	if (c_flag) {
		asynclist_t *asynclist = parse_asynclist(c_flag_script);
		run_asynclist(asynclist, shell_state.display_cmds);
		check_jobs(&shell_state.background_jobs, true);
		return EXIT_SUCCESS;
	}

	loop();
	/* NOTREACHED */
}
