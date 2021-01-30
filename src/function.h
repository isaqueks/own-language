#ifndef FUNCTION_H
#define FUNCTION_H

#include "../libs/list/list.h"
#include "variable.h"
#include "lexer.h"

// #include "context.h"

typedef void (*native_function_handle_t)(void* function_context_ptr);

typedef struct {
    char* name;
    List* arg_models; // List of variables with NULL values
    List* tokens; // Store the code in tokens
    native_function_handle_t native_function;
} function_t;

function_t* function_create(char* name, List* arg_models, List* tokens);

function_t* function_create_empty(char* name, List* arg_models);

function_t* function_create_native(char* name, List* arg_models,
    native_function_handle_t native_handle);

function_t* function_add_code(function_t* fn, List* tokens);

function_t* function_add_token(function_t* fn, token_t* token);

variable_t* function_get_arg(function_t* fn, char* argname);

void function_free(function_t* fn);

#endif