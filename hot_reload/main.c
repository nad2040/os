#include "reloader/hot_reloader.h"
#include "plugin/printer.h"
#include <unistd.h>

int main(void) {
    while (1) {
        hot_loader_t loaded = load_function("build/lib/printer.dylib", "printer");
        printer_t printer = loaded.function;
        printer();
        sleep(8);
        dlclose(loaded.module);
    }

    return 0;
}
