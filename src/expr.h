#ifndef EXPR_H
#define EXPR_H

#include "../libs/list/list.h"
#include "lexer.h"
#include "variable.h"
#include "parser.h"
#include "context.h"
#include "primtypes.h"

typedef struct {
    void* value;
    variable_type_t type;
} expr_value_t;

typedef struct {
    void* value_a;
    void* value_b; // b = NULL = operation_t is not an operation, but value. So value_a is a ptr to expr_value_t
    bool a_is_expr_value;
    bool b_is_expr_value;
    token_type_t operation;
} operation_t;

expr_value_t* expr_value_create(void* value, variable_type_t type);
void expr_value_free(expr_value_t* expr);

operation_t* operation_create(void* a, void* b, bool a_is_expr, bool b_is_expr, token_type_t op);
void operation_free(operation_t* operation);

List* expr_compile(List* tokens, context_t* context, int* out_i);
variable_t* expr_eval_from_compiled(List* compiled_expr, context_t* context);

bool operation_is_value(operation_t* operation);

#endif