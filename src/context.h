#ifndef CONTEXT_H
#define CONTEXT_H

#include "exceptions.h"
#include "variable.h"
#include "../libs/list/list.h"
#include <inttypes.h>

typedef struct {
    uint32_t id;
    int context_level;
    void* parent;
    List* varlist;
    List* children;
} context_t;

extern int global_id;
context_t* context_create(context_t* parent);

void context_free(context_t* context);

variable_t* context_search_variable_unsafe(context_t* context, char* varname);

variable_t* context_search_variable(context_t* context, char* varname);

void context_add_variable(context_t* context, variable_t* var);

#endif