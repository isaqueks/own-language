#ifndef EXPR_H
#define EXPR_H

#include "../libs/list/list.h"
#include "lexer.h"
#include "variable.h"
#include "parser.h"
#include "context.h"
#include "primtypes.h"

typedef enum {

    item_operator,
    item_value,
    item_expr

} expr_item_type_t;

typedef struct {

    void* ptr;
    expr_item_type_t type;

} expr_item_t;

expr_item_t* expr_item_create(void* item_src, expr_item_type_t type);
void expr_item_free(expr_item_t* expr);

List* expr_parse_linear(List* tokens, context_t* context, int* out_i);
List* expr_mount_tree(List* expr, int curr_priority);

void expr_print_linear(List* linear_expr);
void expr_print_tree(List* tree_expr);

int expr_op_get_priority(token_type_t op_type);

#endif