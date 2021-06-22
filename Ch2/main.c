#include "ch2.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    printf("max file descriptors is %ld\n", open_max());
    size_t sz;
    printf("max path %s %zu", path_alloc(&sz), sz);
}