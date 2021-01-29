#ifndef PARSER_H
#define PARSER_H

#include "../libs/list/list.h"
#include "exceptions.h"
#include "lexer.h"
#include "context.h"
#include "variable.h"
#include "function.h"


void parser_parse(char* line, context_t* context);
void parser_parse_tokens(List* tokens, context_t* context);
void parser_function_invoke(function_t* function, context_t* context);

int parser_eval_expr_until_tokens(List* tokens, context_t* context,
    variable_type_t type, int i, token_type_t limits[], int limitscount, void** out, int* size);

int parser_eval_expr(List* tokens, context_t* context,
    variable_type_t type, int i, void** out, int* size);

#endif