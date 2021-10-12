#include "expr.h"

#include "errors.h"
#include "primtypes.h";

#define MAX_OP_PRIORITY 1

int expr_op_get_priority(token_type_t op_type) {
    if (op_type == operation_sub || op_type == operation_sum) {
        return 0;
    } else if (op_type == operation_mul || op_type == operation_div) {
        return 1;
    }
    return -1;
}

expr_item_t *expr_item_create(void *item_src, expr_item_type_t type) {
    expr_item_t *item = (expr_item_t *)malloc(sizeof(expr_item_t));
    item->ptr = item_src;
    item->type = type;
    return item;
}

void expr_item_free(expr_item_t *expr) {
    free(expr->ptr);
    free(expr);
}

List *expr_parse_linear(List *tokens, context_t *context, int *out_i) {
    List *result = create_list(sizeof(expr_item_t), 16);
    int i = *out_i;

    for (; i < tokens->used_length; i++) {
        token_t *token = (token_t *)list_get(tokens, i);
        expr_item_t *item = expr_item_create(NULL, item_value);

        if (token->type == number) {
            item->type = item_value;
            double *value = (double *)malloc(sizeof(double));
            *value = String_toNumber(token->token);
            item->ptr = (void *)value;
        } else if (token->type == operation_sum ||
                   token->type == operation_sub ||
                   token->type == operation_mul ||
                   token->type == operation_div) {
            item->type = item_operator;
            item->ptr = malloc(sizeof(token_type_t));
            *((token_type_t *)item->ptr) = token->type;
        } else {
            RUNTIME_ERR("Not Implemented!", "YET!");
        }

        list_add(result, item);
    }

    *out_i = i;
    return result;
}

List *expr_mount_tree(List *expr, int curr_priority) {
    if (curr_priority < 0) {
        return expr;
    }

    printf("Received ");
    expr_print_tree(expr);
    printf("\n");

    List *result = create_list(sizeof(expr_item_t), 16);

    expr_item_t *first_item = NULL;
    expr_item_t *second_item = NULL;

    for (int i = 0; i < expr->used_length; i++) {
        expr_item_t *item = (expr_item_t *)list_get(expr, i);

        if (first_item == NULL) {
            first_item = item;
        }
        else if (second_item == NULL) {
            second_item = item;

            expr_item_t* term, *op;
            bool first_is_op = false;

            if (first_item->type == item_operator) {
                op = first_item;
                term = second_item;
                first_is_op = true;
            }
            else if (second_item->type == item_operator) {
                op = second_item;
                term = first_item;
            }
            else {
                RUNTIME_ERR(SYNTAX_ERROR, "Operator expected!");
            }

            int priority = expr_op_get_priority(*(token_type_t*)op->ptr);
            if (priority < 0) {
                RUNTIME_ERR(SYNTAX_ERROR, "Invalid operator.");
            }

            if (priority == curr_priority) {
                if (first_is_op) {
                    // <OP> <TERM>
                    // E.g.: + 5

                    // We'll get the last term from the result list,
                    // but if i <= 1, no item was added to result list!
                    if (i <= 1) {
                        RUNTIME_ERR(SYNTAX_ERROR, "A term expected! A OP B");
                    }

                    int result_last_index = result->used_length-1;
                    expr_item_t* last_term_raw = (expr_item_t*)list_get(result, result_last_index);
                    // Backup last item, because it'll be
                    // overwritten with list_set
                    expr_item_t* last_term = expr_item_create(last_term_raw->ptr, last_term_raw->type);

                    List* grouped_term = create_list(sizeof(expr_item_t), 3);
                    list_add(grouped_term, last_term);
                    list_add(grouped_term, op);
                    list_add(grouped_term, term);

                    expr_item_t* group = expr_item_create(grouped_term, item_expr);
                    list_set(result, result_last_index, group);
                }
                else {
                    // <TERM> <OP>
                    // Eg: 5 +
                    if (i >= expr->used_length - 1) {
                        RUNTIME_ERR(SYNTAX_ERROR, "B term expected! A OP B");
                    }

                    expr_item_t* next = list_get(expr, ++i);

                    List* grouped_term = create_list(sizeof(expr_item_t), 3);
                    list_add(grouped_term, term);
                    list_add(grouped_term, op);
                    list_add(grouped_term, next);

                    expr_item_t* group = expr_item_create(grouped_term, item_expr);
                    list_add(result, group);
                }
            }
            else {
                list_add(result, first_item);
                list_add(result, second_item);
            }

            first_item = NULL;
            second_item = NULL;
        }
      
    }

    if (first_item != NULL) {
        list_add(result, first_item);
    }
    if (second_item != NULL) {
        list_add(result, second_item);
    }

    printf("Round (%d)\n\t(res) ", curr_priority);
    expr_print_tree(result);
    printf("\n");

    return expr_mount_tree(result, curr_priority - 1);
}

void expr_print_linear(List *linear_expr) {
    for (int i = 0; i < linear_expr->used_length; i++) {
        expr_item_t *item = (expr_item_t *)list_get(linear_expr, i);
        switch (item->type) {
            case item_operator:
                printf(" OP (%s) ",
                       token_type_str[*((token_type_t *)item->ptr)]);
                break;
            case item_expr:
                printf(" TREE (...) ");
                break;
            case item_value:
                printf(" VALUE (%f) ", *((double *)item->ptr));
                break;
            default:
                RUNTIME_ERR(TYPE_ERROR, "Unknown item type!");
                break;
        }
    }
}

void expr_print_item(expr_item_t *item) {
    if (item->type == item_value) {
        // Let's assume it's a double by now
        printf("%f", *((double *)item->ptr));
    } else if (item->type == item_operator) {
        printf("%s", token_type_str[*((token_type_t *)item->ptr)]);
    } else if (item->type == item_expr) {
        // So, it's a list with the terms
        expr_print_tree((List *)item->ptr);
    }

    printf(" ");
}

void expr_print_tree(List *tree_expr) {
    printf("[", tree_expr->used_length);

    for (int i = 0; i < tree_expr->used_length; i++) {
        printf(" ");

        expr_item_t *item = (expr_item_t *)list_get(tree_expr, i);
        expr_print_item(item);
    }

    printf("]");
}