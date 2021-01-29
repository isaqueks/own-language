#include "function.h"
#include "../libs/list/list.h"
#include "variable.h"
#include <string.h>
#include "exceptions.h"

function_t* function_create(char* name, List* arg_models, int linesOfCode, char* code) {
    function_t* func = malloc(sizeof(function_t));
    func->name = name;
    func->arg_models = arg_models;
    func->inner_code_lines = linesOfCode;
    func->inner_code = code;
    func->native_function = NULL;
}

function_t* function_create_native(char* name, List* arg_models,
native_function_handle_t native_handle) {

    function_t* func = malloc(sizeof(function_t));
    func->name = name;
    func->arg_models = arg_models;
    func->inner_code = NULL;
    func->inner_code_lines = 0;
    func->native_function = native_handle;

}

function_t* function_add_code_line(function_t* fn, char* line) {
    if (fn->inner_code == NULL)
        fn->inner_code = malloc(strlen(line)+2);
    else
        fn->inner_code = realloc(fn->inner_code, strlen(fn->inner_code)+2);
    
    strcat(fn->inner_code, line);
    strcat(fn->inner_code, "\n");

    fn->inner_code_lines++;
    return fn;
}

variable_t* function_get_arg(function_t* fn, char* argname) {
    for (int i = 0; i < fn->arg_models->usedLength; i++) {
        variable_t* arg = list_get(fn->arg_models, i);
        if (strcmp(arg->name, argname) == 0)
            return arg;
    }
    Throw("No such argument for this function.");
}

void function_free(function_t* fn) {
    free(fn->name);
    for (int i = 0; i < fn->arg_models->usedLength; i++) {
        variable_free(*((variable_t**)list_get(fn->arg_models, i)));
    }
    list_free(fn->arg_models);
}