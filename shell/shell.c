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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "dynarr.h"

// https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_09_02

// create command struct
DYNARR(command_t, char *);

// create pipeline of commands
DYNARR(pipeline_t, command_t *);

// not implementing AND-OR list

// async tagged pipeline
typedef struct {
	pipeline_t *p;
	bool async;
} apipeline_t;

// create async list of async tagged pipelines
DYNARR(asynclist_t, apipeline_t);

// dynamic array for the n-1 pipes
DYNARR(pipes_t, int *);
#define READ_END 0
#define WRITE_END 1

// dynamic array for child pids and background pids.
DYNARR(pids_t, pid_t);

#define PROMPT "sish$"

typedef struct {
	pids_t *background_jobs;
	pid_t current_pid;
	char last_exit_status;
	bool display_cmds;
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
display_pipeline(pipeline_t pipeline)
{
	for (int i = 0; i < pipeline.size; ++i) {
		command_t *cmd = pipeline.data[i];
		printf("+ ");
		for (int j = 0; j < cmd->size; ++j) {
			char *arg = cmd->data[j];
			printf("%s ", arg);
		}
		printf("\n");
	}
}

asynclist_t *
parse_commands(char *line)
{
	char *stripped_line = strip(line);

	if (strcmp(stripped_line, "") == 0) { return NULL; }

	DYNARR_INIT(asynclist_t, async_pipeline_list);

	char *pipeline_str;
	while ((pipeline_str = strsep(&stripped_line, "&"))) {
		pipeline_str = strdup(pipeline_str);
		char *stripped_pipeline_str = strip(pipeline_str);

		if (strcmp(stripped_pipeline_str, "") == 0)
			break; // if last command is backgrounded, then there
			       // is no other command to add but there is an
			       // empty string

		DYNARR_INIT(pipeline_t, pipeline);
		char *command_str;
		while ((command_str = strsep(&stripped_pipeline_str, "|"))) {
			command_str = strdup(command_str);
			char *stripped_cmd = strip(command_str);

			DYNARR_INIT(command_t, command);
			char *arg;
			while ((arg = strsep(&stripped_cmd, " \t"))) {
				arg = strdup(arg);
				dynarr_append(command, arg);
			}
			dynarr_append(pipeline, command);

			free(command_str);
		}

		free(pipeline_str);

		apipeline_t async_pipeline
		    = { .p = pipeline, .async = (stripped_line != NULL) };

		dynarr_append(async_pipeline_list, async_pipeline);
	}
	return async_pipeline_list;
}

void
run_commands(asynclist_t *asynclist, bool display_commands)
{
	// 1. create all n-1 pipes
	// 2. for each command, fork
	// 3a. detect redirects
	// 4a. set all redirects
	// 5a. then set all pipes
	// 6a. exec
	// 3b. in parent, close all pipe ends
	// 4b. wait for children

	if (asynclist == NULL) { return; }

	if (display_commands)
		for (int i = 0; i < asynclist->size; ++i)
			display_pipeline(*asynclist->data[i].p);

	// I'm only going to deal with one async command for now
	apipeline_t async_pipeline = asynclist->data[0];
	bool async = async_pipeline.async;
	pipeline_t *cmds = async_pipeline.p;

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
			fprintf(stderr, "couldn't fork: %s\n",
				strerror(errno));
			exit(EXIT_FAILURE);
		}
		else if (pid > 0) { // parent
			if (async)
				dynarr_append(shell_state.background_jobs,
					      pid);
			else dynarr_append(cpids, pid);
			continue;
		}

		// pid == 0 (child)

		command_t *cmd = cmds->data[i];

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
					   O_WRONLY | O_CREAT | O_TRUNC, 0666);
				dup2(out_fd, STDOUT_FILENO);
			}
			else if (strcmp(redirect, ">>") == 0) {
				int out_fd = open(redir_filename,
						  O_WRONLY | O_CREAT, 0666);
				// off_t offset = lseek(out_fd, 0, SEEK_END);
				lseek(out_fd, 0, SEEK_END);
				dup2(out_fd, STDOUT_FILENO);
			}
			else if (strcmp(redirect, "<") == 0) {
				int in_fd
				    = open(redir_filename, O_RDONLY, 0666);
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
		fprintf(stderr, "couldn't exec: %s\n", strerror(errno));
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
		       != cpids->data[i])
			;

		shell_state.last_exit_status = (char) WEXITSTATUS(status);
	}
}

void
check_background_jobs()
{
	// check for background processes by waiting, but don't hang
	int status;
	int i = 0;
	while (i < shell_state.background_jobs->size) {
		pid_t waiting_for = shell_state.background_jobs->data[i];
		pid_t got_it = waitpid(waiting_for, &status, WNOHANG);
		if (got_it == waiting_for) {
			printf("[%d] pid %d finished\n", i + 1, got_it);
			dynarr_pop(shell_state.background_jobs, i);
		}
		else ++i;
	}
}

void
argparse(int argc, char **argv, bool *x_flag, bool *c_flag,
	 char **c_flag_script)
{
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "xc:")) != -1)
		switch (c) {
		case 'x':
			*x_flag = true;
			break;
		case 'c':
			*c_flag = true;
			*c_flag_script = optarg;
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
		default:
			exit(EXIT_FAILURE);
		}
	// printf("xflag: %d ", *x_flag);
	// printf("cflag: %d ", *c_flag);
	// printf("cflag_string: %s\n", *c_flag_script);
}

int
main(int argc, char **argv)
{
	bool x_flag = false;
	bool c_flag = false;
	char *c_flag_script = NULL;

	argparse(argc, argv, &x_flag, &c_flag, &c_flag_script);

	shell_state.display_cmds = x_flag;
	shell_state.current_pid = getpid();
	DYNARR_INIT(pids_t, background_jobs);
	shell_state.background_jobs = background_jobs;

	struct sigaction action = { .sa_handler = sighandler };
	sigaction(SIGINT, &action, NULL); // TODO: Check return
	sigaction(SIGQUIT, &action, NULL);
	sigaction(SIGTSTP, &action, NULL);

	if (c_flag) {
		asynclist_t *asynclist = parse_commands(c_flag_script);
		run_commands(asynclist, shell_state.display_cmds);
		check_background_jobs();
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
		asynclist_t *asynclist = parse_commands(line);
		run_commands(asynclist, shell_state.display_cmds);
		check_background_jobs();
		free(line);
	}
	return 0;
}
