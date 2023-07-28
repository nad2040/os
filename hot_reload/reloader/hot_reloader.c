#include "hot_reloader.h"

hot_loader_t load_function(const char *module_path, const char *function_name) {
    if (!module_path || !function_name) {
        return (hot_loader_t){ .module = NULL, .function = NULL };
    }

    module_t module = dlopen(module_path, RTLD_NOW);
    if (!module) {
        fprintf(stderr, "Failed to get module '%s': %s", module_path, dlerror());
        exit(1);
    }

    function_t function = dlsym(module, function_name);
    if (!function) {
        fprintf(stderr, "Failed to get function '%s': %s", function_name, dlerror());
        exit(1);
    }

    return (hot_loader_t){ .module = module, .function = function };
}
