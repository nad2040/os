#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
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

#define PROMPT "my-sh$"

typedef struct {
	pids_t *background_jobs;
	pid_t current_pid;
	char last_exit_status;
	bool display_cmds;
} shell_state_t;


#endif // SHELL_H
