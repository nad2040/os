#include "hot_reloader.h"

hot_loader_t load_function(const char *module_path, const char *function_name) {
    if (!module_path || !function_name) {
        hot_loader_t t = { .module = NULL, .function = NULL };
        return t;
    }

    void *module = dlopen(module_path, RTLD_NOW);
    if (!module) {
        fprintf(stderr, "Failed to get module '%s': %s", module_path, dlerror());
        exit(1);
    }

    void *function = dlsym(module, function_name);
    if (!function) {
        fprintf(stderr, "Failed to get function '%s': %s", function_name, dlerror());
        exit(1);
    }

    hot_loader_t t = { .module = module, .function = function };
    return t;
}
