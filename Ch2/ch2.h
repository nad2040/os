#pragma once
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

long open_max(void);

char * path_alloc(size_t *sizep);