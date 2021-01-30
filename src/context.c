#include "context.h"

int global_id = 0;
context_t* context_create(context_t* parent) {

    int level = 0;

    context_t* ctx = (context_t*)malloc(sizeof(context_t));

    if (parent != NULL) {
        level = parent->context_level + 1;
        list_add(parent->children, &ctx);
    }

    ctx->children = create_list(sizeof(context_t*), 16);
    ctx->varlist = create_list(sizeof(variable_t*), 16);
    ctx->funclist = create_list(sizeof(function_t*), 16);

    ctx->id = global_id++;
    ctx->context_level = level;
    ctx->parent = parent;

    return ctx;
}

void context_free(context_t* context) {

    for (int i = 0; i < context->varlist->usedLength; i++) {
        variable_free(list_get(context->varlist, i));
    }
    list_free(context->varlist);

    for (int i = 0; i < context->funclist->usedLength; i++) {
        function_free(list_get(context->funclist, i));
    }
    list_free(context->funclist);

    for (int i = 0; i < context->children->usedLength; i++) {
        context_t* child = (context_t*)list_get(context->children, i);
        context_free(child);
    }
    list_free(context->children);

    bool successfullCleaned = false;
    for(int i = 0; i < (((context_t*)(context->parent))->children->usedLength); i++) {
        context_t* ctx = *(context_t**)list_get(((context_t*)(context->parent))->children, i);
        if (ctx->id == context->id) {
            list_remove_at(((context_t*)(context->parent))->children, i);
            successfullCleaned = true;
            break;
        }
    }

    if (!successfullCleaned)
        Throw("Context is lost (NO_PARENT_CONTEXT).");

    free(context);
}

variable_t* context_search_variable_unsafe(context_t* context, char* varname) {

    //printf("[Scope lookup]: %s (%d) <%d>\n", varname, context->id, context->children->usedLength);

    while(context != NULL) {
        for(int i = 0; i < context->varlist->usedLength; i++) {
            variable_t* var = *(variable_t**)list_get(context->varlist, i);
            if (strcmp(var->name, varname) == 0)
                return var;
        }
        context = context->parent;
    }

    

    return NULL;
}

variable_t* context_search_variable (context_t* context, char* varname) {
    variable_t* var = context_search_variable_unsafe(context, varname);
    if (var == NULL)
        Throw("No corresponding variable.");
    return var;
}

void context_add_variable(context_t* context, variable_t* var) {
    list_add(context->varlist, &var);
}


void context_add_function(context_t* context, function_t* func) {
    list_add(context->funclist, &func);
}

function_t* context_search_function_unsafe(context_t* context, char* funcname) {
    //printf("[Scope lookup function]: %s (%d) <%d>\n", funcname, context->id, context->children->usedLength);

    while(context != NULL) {
        for(int i = 0; i < context->funclist->usedLength; i++) {
            function_t* func = *(function_t**)list_get(context->funclist, i);
            if (strcmp(func->name, funcname) == 0)
                return func;
        }
        context = context->parent;
    }

    return NULL;
}

function_t* context_search_function(context_t* context, char* funcname) {
    function_t* fptr = context_search_function_unsafe(context, funcname);
    if (fptr == NULL)
        Throw("No corresponding function.");

    return fptr;
}