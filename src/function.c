#include "function.h"
#include "lexer.h"
#include "../libs/list/list.h"
#include "variable.h"
#include <string.h>
#include "exceptions.h"

function_t* function_create_empty(char* name, List* arg_models) {
        function_t* func = malloc(sizeof(function_t));
    func->name = name;
    func->arg_models = arg_models;
    func->tokens = create_list(sizeof(token_t), 32);
    func->native_function = NULL;
}

function_t* function_create(char* name, List* arg_models, List* tokens) {
    function_t* func = malloc(sizeof(function_t));
    func->name = name;
    func->arg_models = arg_models;
    func->tokens = tokens;
    func->native_function = NULL;
}

function_t* function_create_native(char* name, List* arg_models,
native_function_handle_t native_handle) {

    function_t* func = malloc(sizeof(function_t));
    func->name = name;
    func->arg_models = arg_models;
    func->native_function = native_handle;
    func->tokens = NULL;

}

function_t* function_add_code(function_t* fn, List* tokens) {
    for (int i = 0; i < tokens->usedLength; i++) {
        list_add(fn->tokens, list_get(tokens, i));
    }
    return fn;
}

function_t* function_add_token(function_t* fn, token_t* token) {
    list_add(fn->tokens, token);
    return fn;
}

variable_t* function_get_arg(function_t* fn, char* argname) {
    for (int i = 0; i < fn->arg_models->usedLength; i++) {
        variable_t* arg = *((variable_t**)list_get(fn->arg_models, i));
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