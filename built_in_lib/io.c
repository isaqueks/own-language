#include "io.h"

void lib_print(void* ctxptr)
{
    context_t* ctx = (context_t*)ctxptr;
    variable_t* arg_str = context_search_variable(ctx, "msg");
    void* value = variable_get_value(arg_str);

    switch (arg_str->type) {
        case String:
            printf("%s\n", (char*)value);
            break;
        case Number:
            printf("%f\n", (double)*((double*)value));
            break;
    }
}

void lib_install(context_t* main_context) {
    variable_t* arg_str = variable_create("msg", Any, NULL, 0);
    native_function_handle_t printhandle = lib_print;
    List* args = create_list(sizeof(variable_t*), 1);
    list_add(args, &arg_str);
    function_t* print_func = function_create_native("print", args, printhandle);
    context_add_function(main_context, print_func);
}