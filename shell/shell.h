#ifndef _SISH_H_
#define _SISH_H_

#include <stdbool.h>

#include "dynarr.h"

/*
 * Following the POSIX specification:
 * https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_09_02
 *
 * The general grammar hierarchy is:
 * command
 * pipeline of command
 * and-or list of pipeline
 * async list of and-or list
 *
 * we don't need to deal with and-or lists
 */

DYNARR(args_t, char *);

typedef struct command_t {
	args_t *args;
	char *in_filename;
	char *out_filename;
	bool append;
} command_t;

DYNARR(pipeline_t, command_t *);

/* async tagged pipeline */
typedef struct apipeline_t {
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

#define PROMPT "mysh$"

typedef struct {
	pids_t background_jobs;
	char *full_path;
	pid_t current_pid;
	uint8_t last_exit_status;
	bool display_cmds;
} shell_state_t;

extern shell_state_t shell_state;

void args_free(args_t *);
void pipeline_free(pipeline_t *);
void asynclist_free(asynclist_t *);

void display_pipeline(pipeline_t *);

command_t *parse_command(char *);
pipeline_t *parse_pipeline(char *);
asynclist_t *parse_asynclist(char *);

void handle_pipeline_redirection(pipeline_t *, int, const pipes_t);
void handle_file_redirection(pipeline_t *, int);

void check_jobs(pids_t *, bool);

void run_pipeline(pipeline_t *, bool);
void run_asynclist(asynclist_t *, bool);

#endif /* _SHELL_H_ */
