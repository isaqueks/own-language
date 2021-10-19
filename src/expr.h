#ifndef EXPR_H
#define EXPR_H

#include "../libs/list/list.h"
#include "lexer.h"
#include "variable.h"
#include "parser.h"
#include "context.h"
#include "primtypes.h"

#define MAX_OP_PRIORITY 1

typedef enum {

    item_operator,
    item_value,
    item_expr

} expr_item_type_t;

typedef struct {

    void* ptr;
    variable_type_t value_type;
    expr_item_type_t type;

} expr_item_t;

expr_item_t* expr_item_create(void* item_src, expr_item_type_t type, variable_type_t val_type);
void expr_item_free(expr_item_t* expr);

List* expr_parse_linear(List* tokens, context_t* context, int* out_i, int opened_parenthesis, int max_i);
List* expr_mount_tree(List* expr, int curr_priority);

void expr_print_item(expr_item_t *item);
void expr_print_linear(List* linear_expr);
void expr_print_tree(List* tree_expr);

int expr_op_get_priority(token_type_t op_type);

#endif