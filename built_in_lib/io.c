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