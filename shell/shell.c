#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "dynarr.h"
// create command struct
DYNARR(cmd_t, char *);
// create pipeline of commands.
DYNARR(pipeline_t, cmd_t *);

// dynamic array for the n-1 pipes
DYNARR(pipes_t, int *);
#define READ_END 0
#define WRITE_END 1

// dynamic array for child pids
DYNARR(pids_t, pid_t);

#define PROMPT "sish$"

typedef struct {
	pid_t current_pid;
	int last_exit_status;
} shell_state_t;

shell_state_t shell_state = { 0 };

volatile sig_atomic_t interrupted = false;
void
sighandler(int signum)
{
	interrupted = true;
}

const char *redirects[3] = { "<", ">>", ">" };

/**
 * sets trailing whitespace to null bytes and
 * returns pointer to first non-whitespace char.
 * do not lose original pointer when freeing.
 */
char *
strip(char *string)
{
	char *begin = string;
	char *end = string + strlen(string) - 1;
	while (isspace((int) *begin))
		begin++;
	while (isspace((int) *end)) {
		*end = '\0';
		end--;
	}
	return begin;
}

void
display_pipeline(pipeline_t *pipeline)
{
	for (int i = 0; i < pipeline->size; ++i) {
		cmd_t *cmd = pipeline->data[i];
		printf("+ ");
		for (int j = 0; j < cmd->size; ++j) {
			char *arg = cmd->data[j];
			printf("%s ", arg);
		}
		printf("\n");
	}
}

pipeline_t *
parse_commands(char *line)
{
	char *stripped_line = strip(line);

	if (strcmp(stripped_line, "") == 0) { return NULL; }

	// tokenize - using dynamic array
	DYNARR_INIT(pipeline_t, pipeline);

	char *command, *arg;
	while ((command = strsep(&stripped_line, "|"))) {
		command = strdup(command);
		char *stripped_cmd = strip(command);

		DYNARR_INIT(cmd_t, cmd);
		while ((arg = strsep(&stripped_cmd, " \t"))) {
			arg = strdup(arg);
			dynarr_append(cmd, arg);
		}
		dynarr_append(pipeline, cmd);

		free(command);
	}
	return pipeline;
}

void
run_commands(pipeline_t *cmds)
{
	// 1. create all n-1 pipes
	// 2. for each command, fork
	// 3. detect redirects and background.
	// 4. set all redirects
	// 5. then set all pipes
	// 6. exec

	if (cmds == NULL) { return; }

	DYNARR_INIT(pipes_t, pipes);
	for (int i = 0; i < cmds->size - 1; ++i) {
		int *new_pipe = malloc(sizeof(int[2]));
		if (new_pipe == NULL) {
			fprintf(stderr, "couldn't init pipe #%d\n", i + 1);
			exit(EXIT_FAILURE);
		}
		pipe(new_pipe); // TODO: Check return
		dynarr_append(pipes, new_pipe);
	}

	DYNARR_INIT(pids_t, cpids);
	for (int i = 0; i < cmds->size; ++i) {
		// fork
		pid_t pid = fork();
		if (pid < 0) {
			fprintf(stderr, "couldn't fork %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		else if (pid > 0) { // parent
			dynarr_append(cpids, pid);
			continue;
		}

		// pid == 0 (child)

		cmd_t *cmd = cmds->data[i];

		// redirects
		// printf("redirects\n");
		int j = 0;
		while (j < cmd->size) {
			char *redir_filename = NULL;
			int k;
			for (k = 0; k < 3; ++k) {
				if (strcmp(cmd->data[j], redirects[k]) == 0) {
					free(cmd->data[j]);
					dynarr_pop(cmd, j);
					redir_filename = cmd->data[j];
					dynarr_pop(cmd, j);
					break;
				}
				else if (strncmp(cmd->data[j], redirects[k],
						 strlen(redirects[k]))
					 == 0) {
					redir_filename
					    = strdup(cmd->data[j]
						     + strlen(redirects[k]));
					free(cmd->data[j]);
					dynarr_pop(cmd, j);
					break;
				}
			}
			if (redir_filename == NULL) {
				j++;
				continue;
			}
			const char *redirect = redirects[k];
			// TODO: handle the redirect types and CHECK FD RETURN
			// VALUE
			if (strcmp(redirect, ">") == 0) {
				int out_fd
				    = open(redir_filename,
					   O_WRONLY | O_CREAT | O_TRUNC);
				dup2(out_fd, STDOUT_FILENO);
			}
			else if (strcmp(redirect, ">>") == 0) {
				int out_fd
				    = open(redir_filename, O_WRONLY | O_CREAT);
				// off_t offset = lseek(out_fd, 0, SEEK_END);
				lseek(out_fd, 0, SEEK_END);
				dup2(out_fd, STDOUT_FILENO);
			}
			else if (strcmp(redirect, "<") == 0) {
				int in_fd = open(redir_filename, O_RDONLY);
				dup2(in_fd, STDIN_FILENO);
			}
			free(redir_filename);
		}

		// pipes
		// close all other pipe ends
		// TODO: check close and dup2 return value
		for (int p = 0; p < cmds->size - 1; ++p) {
			// printf("proc %d: pipe %d\n", i, p);
			if (p != i) {
				// printf("for proc %d: closed W%d\n", i, p);
				close(pipes->data[p][WRITE_END]);
			}
			if (p != i - 1) {
				// printf("for proc %d: closed R%d\n", i, p);
				close(pipes->data[p][READ_END]);
			}
		}
		// pipe redirects for current proc i
		if (i != cmds->size - 1) {
			// printf("for proc %d: WRITE_END of pipe %d is now "
			       // "pointed to by STDOUT_FILENO\n",
			       // i, i);
			dup2(pipes->data[i][WRITE_END], STDOUT_FILENO);
		}
		if (i != 0) {
			// printf("for proc %d: READ_END of pipe %d is now "
			//        "pointed to by STDIN_FILENO\n",
			//        i, i - 1);
			dup2(pipes->data[i - 1][READ_END], STDIN_FILENO);
		}

		// exec
		// printf("exec\n");
		// TODO: exec impl. need special cases for builtins!
		execvp(cmd->data[0], cmd->data);
		fprintf(stderr, "exec failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// close all pipes in parent
	for (int i = 0; i < pipes->size; ++i) {
		close(pipes->data[i][READ_END]);
		close(pipes->data[i][WRITE_END]);
	}

	// wait for children
	// printf("waiting for children\n");
	int status;
	for (int i = 0; i < cpids->size; ++i) {
		pid_t pid;
		// printf("waiting for child %d\n", cpids->data[i]);
		while ((pid = waitpid(cpids->data[i], &status, 0))
		       != cpids->data[i]);
	}
}

int
main(int argc, char **argv)
{
	shell_state.current_pid = getpid();

	bool x_flag = false;
	bool c_flag = false;
	char *c_flag_script = NULL;
	int c;

	opterr = 0;

	while ((c = getopt(argc, argv, "xc:")) != -1)
		switch (c) {
		case 'x':
			x_flag = true;
			break;
		case 'c':
			c_flag_script = optarg;
			break;
		case '?':
			if (optopt == 'c')
				fprintf(stderr,
					"Option -%c requires an argument.\n",
					optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n",
					optopt);
			else
				fprintf(stderr,
					"Unknown option character `\\x%x'.\n",
					optopt);
			return 1;
		default:
			exit(EXIT_FAILURE);
		}

	printf("xflag: %d ", x_flag);
	printf("cflag: %d ", c_flag);
	printf("cflag_string: %s\n", c_flag_script);

	struct sigaction action = { .sa_handler = sighandler };
	sigaction(SIGINT, &action, NULL); // TODO: Check return
	sigaction(SIGQUIT, &action, NULL);
	sigaction(SIGTSTP, &action, NULL);

	if (c_flag) {
		pipeline_t *pipeline = parse_commands(c_flag_script);
		run_commands(pipeline);
		return 0;
	}

	char *line;
	size_t len = 0;
	while (1) {
		line = NULL;
		printf(PROMPT " ");
		int nread = getline(&line, &len,
				    stdin); // TODO: Maybe stop ctrl-D and only
					    // allow `quit` as exit method
		if (nread == -1) {
			if (interrupted) {
				printf("\n");
				interrupted = false;
				continue;
			}
			break;
		}
		pipeline_t *pipeline = parse_commands(line);
		run_commands(pipeline);

		free(line);
	}
	return 0;
}
