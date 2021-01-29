#ifndef FUNCTION_H
#define FUNCTION_H

#include "../libs/list/list.h"
#include "variable.h"
// #include "context.h"

typedef void (*native_function_handle_t)(void* function_context_ptr);

typedef struct {
    char* name;
    List* arg_models; // List of variables with NULL values
    char* inner_code;
    int inner_code_lines;
    native_function_handle_t native_function;
} function_t;

function_t* function_create(char* name, List* arg_models, int linesOfCode, char* code);

function_t* function_create_native(char* name, List* arg_models,
    native_function_handle_t native_handle);

function_t* function_add_code_line(function_t* fn, char* line);

variable_t* function_get_arg(function_t* fn, char* argname);

void function_free(function_t* fn);

#endif