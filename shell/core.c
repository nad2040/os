#include <sys/wait.h>

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "shell.h"
#include "util.h"

/*
 * safely deallocate args_t *
 */
void
args_free(args_t *args)
{
	for (int i = 0; i < args->size; ++i) {
		free(args->data[i]);
		args->data[i] = NULL;
	}
	free(args->data);
	args->data = NULL;
}

/*
 * safely deallocate pipeline_t *
 */
void
pipeline_free(pipeline_t *pipeline)
{
	for (int i = 0; i < pipeline->size; ++i) {
		args_free(pipeline->data[i]->args);
		pipeline->data[i]->args = NULL;
		if (pipeline->data[i]->in_filename != NULL) {
			free(pipeline->data[i]->in_filename);
			pipeline->data[i]->in_filename = NULL;
		}
		if (pipeline->data[i]->out_filename != NULL) {
			free(pipeline->data[i]->out_filename);
			pipeline->data[i]->out_filename = NULL;
		}
	}
	free(pipeline->data);
	pipeline->data = NULL;
}

/*
 * safely deallocate asynclist_t *
 */
void
asynclist_free(asynclist_t *asynclist)
{
	for (int i = 0; i < asynclist->size; ++i) {
		pipeline_free(asynclist->data[i].p);
		asynclist->data[i].p = NULL;
	}
	free(asynclist->data);
	asynclist->data = NULL;
}

/*
 * display the commands that are run
 */
void
display_pipeline(pipeline_t *pipeline)
{
	for (int i = 0; i < pipeline->size; ++i) {
		args_t *args = pipeline->data[i]->args;
		(void)printf("+ ");
		for (int j = 0; j < args->size; ++j) {
			char *arg = args->data[j];
			(void)printf("%s ", arg);
		}
		(void)printf("\n");
	}
}

/*
 * parse_command allocates an args_t dynarr on heap and with string arguments
 * tokenized by strtok_r. Then it determines the redirects for such a command
 * and modifies the args appropriately.
 */
command_t *
parse_command(char *stripped_command_str)
{
	static const char *const redirects[3] = {"<", ">>", ">"};

	char *arg;
	char *strtok_ctx;
	args_t *args;

	DYNARR_ALLOC(args);

	for (arg = strtok_r(stripped_command_str, " \t", &strtok_ctx);
	     arg != NULL; arg = strtok_r(NULL, " \t", &strtok_ctx)) {
		arg = strip_copy(arg);
		dynarr_append(args, arg);
	}

	char *in_filename = NULL;
	char *out_filename = NULL;
	bool append = false;

	int arg_index = 0;
	while (arg_index < args->size) {
		char *redir_filename = NULL;
		int redir_index = 0;

		while (redir_index < 3) {
			if (strcmp(args->data[arg_index],
			           redirects[redir_index]) == 0) {
				/* redirection is in two separate arguments */
				free(args->data[arg_index]);
				dynarr_pop(args, arg_index);
				if (arg_index == args->size) {
					warnx("redirect missing argument");
					args_free(args);
					free(in_filename);
					free(out_filename);
					return NULL;
				}
				redir_filename = args->data[arg_index];
				dynarr_pop(args, arg_index);
				break;
			} else if (strncmp(args->data[arg_index],
			                   redirects[redir_index],
			                   strlen(redirects[redir_index])) ==
			           0) {
				/* redirection is a single argument */
				redir_filename =
				    strdup(args->data[arg_index] +
				           strlen(redirects[redir_index]));
				free(args->data[arg_index]);
				dynarr_pop(args, arg_index);
				break;
			}
			++redir_index;
		}
		if (redir_filename == NULL) {
			++arg_index;
			continue;
		}
		const char *redirect = redirects[redir_index];
		if (strcmp(redirect, ">") == 0) {
			free_and_copy(&out_filename, redir_filename);
			append = false;
		} else if (strcmp(redirect, ">>") == 0) {
			free_and_copy(&out_filename, redir_filename);
			append = true;
		} else if (strcmp(redirect, "<") == 0) {
			free_and_copy(&in_filename, redir_filename);
		}
		free(redir_filename);
	}

	command_t *command;

	if ((command = calloc(1, sizeof(command_t))) == NULL) {
		err(EXIT_FAILURE, "failed to alloc command");
	}

	*command = (command_t){.args = args,
	                       .in_filename = in_filename,
	                       .out_filename = out_filename,
	                       .append = append};

	return command;
}

/*
 * parse_pipeline allocates a pipeline_t dynarr on heap
 * and calls parse_command for each command.
 */
pipeline_t *
parse_pipeline(char *stripped_pipeline_str)
{
	char *command_str, *stripped_command_str;
	char *strtok_ctx;
	command_t *command;
	pipeline_t *pipeline;

	DYNARR_ALLOC(pipeline);

	if (stripped_pipeline_str[0] == '|' ||
	    stripped_pipeline_str[strlen(stripped_pipeline_str) - 1] == '|') {
		warnx("syntax error near `|'");
		pipeline_free(pipeline);
		return NULL;
	}

	for (command_str = strtok_r(stripped_pipeline_str, "|", &strtok_ctx);
	     command_str != NULL;
	     command_str = strtok_r(NULL, "|", &strtok_ctx)) {
		stripped_command_str = strip_copy(command_str);

		command = parse_command(stripped_command_str);
		if (command == NULL) {
			pipeline_free(pipeline);
			return NULL;
		}
		dynarr_append(pipeline, command);

		free(stripped_command_str);
	}
	return pipeline;
}

/*
 * parse_asynclist allocates an apipeline_t dynarr on heap.
 * it returns the top level dynarr structure for the input.
 */
asynclist_t *
parse_asynclist(char *stripped_line)
{
	char *pipeline_str, *stripped_pipeline_str;
	char *strtok_ctx;
	pipeline_t *pipeline;
	asynclist_t *async_pipeline_list;

	DYNARR_ALLOC(async_pipeline_list);

	/*
	 * I can't think of any other way to determine if
	 * the last command had an & or not except maybe using
	 * strsep
	 */
	bool last_is_async = (stripped_line[strlen(stripped_line) - 1] == '&');

	for (pipeline_str = strtok_r(stripped_line, "&", &strtok_ctx);
	     pipeline_str != NULL;
	     pipeline_str = strtok_r(NULL, "&", &strtok_ctx)) {
		stripped_pipeline_str = strip_copy(pipeline_str);

		pipeline = parse_pipeline(stripped_pipeline_str);
		if (pipeline == NULL) {
			asynclist_free(async_pipeline_list);
			return NULL;
		}
		apipeline_t async_pipeline = {.p = pipeline, .async = true};
		dynarr_append(async_pipeline_list, async_pipeline);

		free(stripped_pipeline_str);
	}

	if (!last_is_async) {
		async_pipeline_list->data[async_pipeline_list->size - 1].async =
		    false;
	}

	return async_pipeline_list;
}

/* handles pipe redirection for command at given index */
void
handle_pipeline_redirection(pipeline_t *pipeline, int command_index,
                            const pipes_t pipes)
{
	for (int p = 0; p < pipeline->size - 1; ++p) {
		if (p != command_index) {
			(void)close(pipes.data[p][WRITE_END]);
		}
		if (p != command_index - 1) {
			(void)close(pipes.data[p][READ_END]);
		}
	}
	if (command_index != pipeline->size - 1) {
		if (dup2(pipes.data[command_index][WRITE_END], STDOUT_FILENO) ==
		    -1) {
			err(EXIT_FAILURE, "pipe creation dup2");
		}
	}
	if (command_index != 0) {
		if (dup2(pipes.data[command_index - 1][READ_END],
		         STDIN_FILENO) == -1) {
			err(EXIT_FAILURE, "pipe creation dup2");
		}
	}
}

/* handles file redirection for command at given index */
void
handle_file_redirection(pipeline_t *pipeline, int command_index)
{
	bool append = pipeline->data[command_index]->append;
	char *out_filename = pipeline->data[command_index]->out_filename;
	char *in_filename = pipeline->data[command_index]->in_filename;

	int in_fd, out_fd;

	if (out_filename != NULL) {
		int flags =
		    O_WRONLY | O_CREAT | ((append) ? O_APPEND : O_TRUNC);

		if ((out_fd = open(out_filename, flags, 0666)) == -1) {
			err(EXIT_FAILURE, "failed to open %s", out_filename);
		}
		if (dup2(out_fd, STDOUT_FILENO) == -1) {
			err(EXIT_FAILURE, "failed to dup2");
		}
	}
	if (in_filename != NULL) {
		if ((in_fd = open(in_filename, O_RDONLY, 0666)) == -1) {
			err(EXIT_FAILURE, "failed to open %s", in_filename);
		}
		if (dup2(in_fd, STDIN_FILENO) == -1) {
			err(EXIT_FAILURE, "failed to dup2");
		}
	}
}

/*
 * check_jobs checks for finished processes in a list of pids.
 * if background is set to true, check with no-hang flag
 */
void
check_jobs(pids_t *pids, bool background)
{
	pid_t pid;
	int i = 0;
	while (i < pids->size) {
		int status = 0;
		pid_t waiting_for = pids->data[i];
		int flags = (background) ? WNOHANG : 0;

		while ((pid = waitpid(waiting_for, &status, flags)) !=
		           waiting_for &&
		       pid != 0) {
			continue;
		}
		if (pid == waiting_for) {
			if (background) {
				(void)printf("+ done %d\n", pid);
			}
			dynarr_pop(pids, i);
		} else {
			++i;
		}
		/* not handling IFSTOPPED */
		shell_state.last_exit_status =
		    (WIFEXITED(status)) ? (unsigned char)WEXITSTATUS(status)
					: 128 + WTERMSIG(status);
	}
}

/*
 * run_pipeline sets up the pipeline and runs each command.
 */
void
run_pipeline(pipeline_t *pipeline, bool async)
{
	pipes_t pipes = {0};
	pids_t child_pids = {0};

	for (int i = 0; i < pipeline->size - 1; ++i) {
		int *new_pipe = malloc(sizeof(int[2]));
		if (new_pipe == NULL) {
			err(EXIT_FAILURE, "couldn't init pipe memory %d",
			    i + 1);
		}
		if (pipe(new_pipe) == -1) {
			err(EXIT_FAILURE, "couldn't create pipe %d", i + 1);
		}
		dynarr_append(&pipes, new_pipe);
	}

	for (int i = 0; i < pipeline->size; ++i) {
		command_t *cmd;
		args_t *args;

		cmd = pipeline->data[i];
		args = cmd->args;

		/* if no pipes, do cd and exit in the shell process */
		if (pipeline->size == 1) {
			if (strcmp(args->data[0], "cd") == 0) {
				int ret = cd(args->size, args->data);
				shell_state.last_exit_status = ret;
				continue;
			} else if (strcmp(args->data[0], "exit") == 0) {
				exit(EXIT_SUCCESS);
			}
		}

		pid_t pid = fork();
		if (pid < 0) {
			err(EXIT_FAILURE, "couldn't fork");
		} else if (pid > 0) {
			if (async) {
				dynarr_append(&shell_state.background_jobs,
				              pid);
			} else {
				dynarr_append(&child_pids, pid);
			}
			/* parent */
			continue;
		}

		handle_pipeline_redirection(pipeline, i, pipes);

		handle_file_redirection(pipeline, i);

		if (strcmp(args->data[0], "echo") == 0) {
			echo(args->size, args->data);
		} else if (strcmp(args->data[0], "exit") == 0) {
			exit(EXIT_SUCCESS);
		} else {
			execvp(args->data[0], args->data);
			fprintf(stderr, "%s: command not found\n", args->data[0]);
			exit(127);
		}
	}

	for (int i = 0; i < pipes.size; ++i) {
		(void)close(pipes.data[i][READ_END]);
		(void)close(pipes.data[i][WRITE_END]);
	}

	check_jobs(&child_pids, false);
	free(child_pids.data);
	free(pipes.data);
}

/*
 * run_asynclist executes the parsed input one async pipeline at a time.
 */
void
run_asynclist(asynclist_t *asynclist, bool display_commands)
{

	if (display_commands) {
		for (int i = 0; i < asynclist->size; ++i) {
			display_pipeline(asynclist->data[i].p);
		}
	}

	for (int i = 0; i < asynclist->size; ++i) {
		apipeline_t async_pipeline = asynclist->data[i];
		bool async = async_pipeline.async;
		pipeline_t *pipeline = async_pipeline.p;

		run_pipeline(pipeline, async);
	}
}
