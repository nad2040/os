#ifndef HOT_RELOADER_H
#define HOT_RELOADER_H

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

typedef void (*module_t)(void);
typedef struct {
    module_t module;
    void *function;
} hot_loader_t;
hot_loader_t load_function(const char *, const char *);

#endif
