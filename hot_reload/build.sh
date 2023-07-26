#!/bin/sh

CFLAGS="-Wall -Werror"

# hot reloader
gcc-13 -c $CFLAGS reloader/hot_reloader.c -o build/lib/libhot_rldr.o
ar rcs build/lib/libhot_rldr.a build/lib/libhot_rldr.o

# plugin
gcc-13 -shared -fPIC $CFLAGS plugin/printer.c -o build/lib/printer.dylib

# main
gcc-13 -c $CFLAGS main.c -o build/main.o

# link
gcc-13 -Lbuild/lib -lhot_rldr build/main.o -o bin/hot_reload_test
