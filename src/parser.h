#ifndef PARSER_H
#define PARSER_H

#include "../libs/list/list.h"
#include "exceptions.h"
#include "lexer.h"
#include "context.h"
#include "variable.h"
#include "function.h"

typedef enum
{
    no_pending_task,
    func_creation_awaiting_opening_bracket,
    func_creation_awaiting_end,

    conditional_awaiting_opening_bracket,
    conditional_awaiting_end,
} parser_state_task_t;

typedef struct {
    parser_state_task_t task;
    double flag;
    void* memory;
} parser_state_t;

void parser_parse(List* state, char* line, context_t* context);
void parser_parse_tokens(List* state, List* tokens, context_t* context);
void parser_function_invoke(List* state, function_t* function, context_t* context);

int parser_eval_expr_until_tokens(List* state, List* tokens, context_t* context,
    variable_type_t type, int i, token_type_t limits[], int limitscount, void** out, int* size, variable_type_t* out_type);

int parser_eval_expr(List* state, List* tokens, context_t* context,
    variable_type_t type, int i, void** out, int* size, variable_type_t* out_type);

#endif