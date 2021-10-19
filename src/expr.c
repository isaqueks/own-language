#include "expr.h"

#include "errors.h"
#include "primtypes.h";

int expr_op_get_priority(token_type_t op_type) {
    if (op_type == operation_sub || op_type == operation_sum) {
        return 0;
    } else if (op_type == operation_mul || op_type == operation_div) {
        return 1;
    }
    return -1;
}

expr_item_t *expr_item_create(void *item_src, expr_item_type_t type, variable_type_t val_type) {
    expr_item_t *item = (expr_item_t *)malloc(sizeof(expr_item_t));
    if (type < 0 || type > 2) {
        RUNTIME_ERR(TYPE_ERROR, "Unknown type!");
    }
    item->ptr = item_src;
    item->type = type;
    item->value_type = val_type;
    return item;
}

void expr_item_free(expr_item_t *expr) {
    free(expr->ptr);
    free(expr);
}

List *expr_parse_linear(List *tokens, context_t *context, int *out_i, int opened_parenthesis, int max_i) {
    List *result = create_list(sizeof(expr_item_t), 16);
    int i = *out_i;

    for (; i < tokens->used_length && (max_i <= 0 || i <= max_i); i++) {
        token_t *token = (token_t *)list_get(tokens, i);
        expr_item_t *item = expr_item_create(NULL, item_value, Any);

        if (token->type == number) {
            item->type = item_value;
            double *value = (double *)malloc(sizeof(double));
            String_toNumber(token->token, value);
            item->ptr = (void *)value;
            item->value_type = Number;
        } 
        else if (token->type == string_literal) {
            item->type = item_value;
            int len = strlen(token->token);
            item->ptr = malloc(len+1);
            strcpy(item->ptr, token->token);
            item->value_type = String;
        }
        else if (token->type == var_name) {
            item->type = item_value;
            variable_t* var = context_search_variable(context, token->token);
            item->ptr = variable_get_value(var);
            item->value_type = var->type;
        }
        else if (token->type == operation_sum ||
                   token->type == operation_sub ||
                   token->type == operation_mul ||
                   token->type == operation_div) {
            item->type = item_operator;
            token_type_t* token_type_ptr = malloc(sizeof(token_type_t));
            *token_type_ptr = token->type;
            item->ptr = (void*)token_type_ptr;
        } 
        else if (token->type == open_parenthesis) {
            *out_i = i+1;
            List* linear_inside_parenthesis = expr_parse_linear(tokens, context, out_i, 1, max_i);
            i = (*out_i);
            List* tree = expr_mount_tree(linear_inside_parenthesis, MAX_OP_PRIORITY);
            expr_item_free(item);
            
            // Now, merge the lists
            for (int j = 0; j < tree->used_length; j++) {
                list_add(result, list_get(tree, j));
            }
            list_free(tree);
            continue;
        }
        else if (token->type <= close_parenthesis) {
            opened_parenthesis--;
            // Not used
            // TODO: Only alloc if needed
            expr_item_free(item);
            if (opened_parenthesis == 0) {
                *out_i = i;
                return result;
            }
            continue;
        }
        else {
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

            if (term->type != item_expr && term->type != item_value) {
                RUNTIME_ERR(SYNTAX_ERROR, "Value or expr expected");
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
                    expr_item_t* last_term = expr_item_create(last_term_raw->ptr, last_term_raw->type, last_term_raw->value_type);
                    // We won't call expr_item_free
                    // because it also frees the ptr,
                    // which is used in the backup item
                    // free(last_term_raw);

                    List* grouped_term = create_list(sizeof(expr_item_t), 3);
                    list_add(grouped_term, last_term);
                    list_add(grouped_term, op);
                    list_add(grouped_term, term);

                    expr_item_t* group = expr_item_create(grouped_term, item_expr, Any);
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

                    expr_item_t* group = expr_item_create(grouped_term, item_expr, Any);
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

    list_free(expr);

    return expr_mount_tree(result, curr_priority - 1);
}

void expr_print_linear(List *linear_expr) {
    for (int i = 0; i < linear_expr->used_length; i++) {
        expr_item_t *item = (expr_item_t *)list_get(linear_expr, i);
        switch (item->type) {
            case item_operator:
                printf(" OP ");
                break;
            case item_expr:
                printf(" EXPR ");
                break;
            case item_value:
                printf(" VALUE ");
                break;
            default:
                RUNTIME_ERR(TYPE_ERROR, "Unknown item type!");
                break;
        }
    }
}

void expr_print_item(expr_item_t *item) {
    if (item->type == item_value) {
        switch (item->value_type)
        {
            case Number:
                printf("%f", *((double *)item->ptr));
                break;
            case String:
                printf("%s", item->ptr);
                break;
            
            default:
                printf("NOT_IMPL");
                break;
        }
    } else if (item->type == item_operator) {
        printf("%s", token_type_str[*((token_type_t *)item->ptr)]);
    } else if (item->type == item_expr) {
        // So, it's a list with the terms
        expr_print_tree((List *)item->ptr);
    }
    else {
        printf("Type: %d\n", item->type);
        RUNTIME_ERR(TYPE_ERROR, "Unknown expr_item type.");
    }
}

void expr_print_tree(List *tree_expr) {
    printf("[");

    for (int i = 0; i < tree_expr->used_length; i++) {
        printf(" ");

        expr_item_t *item = (expr_item_t *)list_get(tree_expr, i);
        expr_print_item(item);

        printf(" ");
    }

    printf("]");
}