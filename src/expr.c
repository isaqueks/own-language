#include "expr.h"
#include "errors.h"

// #define NEXT_TOKEN()                                   \
//     {                                                  \
//         if (*out_i + 1 == tokens->usedLength)               \
//             ERR(SYNTAX_ERROR, "Expression required."); \
//         token = list_get(tokens, ++(*out_i));                 \
//     }

expr_value_t* expr_value_create(void* value, variable_type_t type) {
    expr_value_t* expr = malloc(sizeof(expr_value_t));
    expr->type = type;
    expr->value = value;
}

void expr_value_free(expr_value_t* expr) {
    free(expr->value);
    free(expr);
}

operation_t* operation_create(void* a, void* b, bool a_is_expr, bool b_is_expr, token_type_t op) {
    operation_t* result = (operation_t*)malloc(sizeof(operation_t));
    result->value_a = a;
    result->value_b = b;
    result->a_is_expr_value = a_is_expr;
    result->b_is_expr_value = b_is_expr;
    result->operation = op;
    return result;
}

bool operation_is_value(operation_t* operation) {
    return operation->value_b == NULL;
}

void operation_free(operation_t* operation) {
    if (operation->a_is_expr_value) {
        expr_value_free(operation->value_a);
    }
    else {
        operation_free(operation->value_a);
    }

    if (operation->b_is_expr_value) {
        expr_value_free(operation->value_b);
    }
    else {
        operation_free(operation->value_b);
    }
    free(operation);
}

void* expr_get_value_number(char* str) {
    double val = String_toNumber(str);
    void* ptr = malloc(sizeof(double));
    memcpy(ptr, &val, sizeof(double));
    return ptr;
}

void* expr_get_value_string(char* str) {
    char* result_str = malloc(sizeof(str)+1);
    strcpy(result_str, str);
    return result_str;
}

void* expr_get_value(token_t* token, context_t* context, variable_type_t* out_type) {
    if (token->type == number) {

        *out_type = Number;
        return expr_get_value_number(token->token);

    } else if (token->type == string_literal) {

        *out_type = String;
        return expr_get_value_string(token->token);

    } else if (token->type == var_name) {

        variable_t* var = context_search_variable(context, token->token);
        void* value = variable_get_value(var);

        if (var->type == Number) {

            *out_type = Number;
            double num = *((double*)value);
            double* return_value = malloc(sizeof(double));
            *return_value = num;
            return return_value;

        } else if (var->type == String) {

            *out_type = String;
            return expr_get_value_string(value);

        } else {
            RUNTIME_ERR(TYPE_ERROR, "Not implemented");
        }

    } else {
        int i = -1;
        ERR(TYPE_ERROR, "number, string_literal or var_name expected!");
    }
    return NULL;
}

List* expr_compile(List* tokens, context_t* context, int* out_i) {

    int i = *out_i;

    // List of operations
    List* result = create_list(sizeof(operation_t), 4);
    token_t* token = list_get(tokens, i);

    operation_t* curr_operation = operation_create(NULL, NULL, false, false, UNKNOWN);
    int operation_element_index = 0;

    for (; i < tokens->usedLength; i++) {
        token = list_get(tokens, i);
        bool is_value = false;
        if (token->type == var_name || token->type == string_literal || token->type == number) {
            is_value = true;
        }

        if (is_value) {
            variable_type_t out_type;
            void* value = expr_get_value(token, context, &out_type);

            if (operation_element_index == 0) {
                curr_operation->value_a = expr_value_create(value, out_type);
                curr_operation->a_is_expr_value = true;
            } else if (operation_element_index == 2) {
                curr_operation->value_b = expr_value_create(value, out_type);
                curr_operation->b_is_expr_value = true;
            } else {
                ERR(SYNTAX_ERROR, "Operator expected!");
            }
            operation_element_index++;

        } else {
            if (token->type >= operation_sum && token->type <= cond_less_or_equal) {
                if (operation_element_index == 1 || (result->usedLength > 0 && operation_element_index == 0)) {

                    curr_operation->operation = token->type;

                    if (operation_element_index == 0) {
                        curr_operation->value_a = list_get(result, result->usedLength-1);
                        // Its not expr_value_t, its operation_t
                        curr_operation->a_is_expr_value = false; 
                        operation_element_index++;
                    }

                } else {
                    ERR(SYNTAX_ERROR, "Something got wrong. Operator expected!");
                }
                operation_element_index++;
            } else {
                ERR(SYNTAX_ERROR, "Operator expected!");
            }
        }

        if (operation_element_index == 3 || i >= tokens->usedLength-1) {
            list_add(result, curr_operation);
            if (i < tokens->usedLength-1) {
                curr_operation = operation_create(NULL, NULL, false, false, UNKNOWN);
            }
            operation_element_index = 0;
        }
    }

    *out_i = i;
    return result;
}


expr_value_t* expr_solve_operation(operation_t* operation) {
    expr_value_t* value_a = NULL;
    expr_value_t* value_b = NULL;

    variable_type_t result_type;

    void* result;

    if (operation->a_is_expr_value) {
        value_a = operation->value_a;
    } else {
        value_a = expr_solve_operation((operation_t*)(operation->value_a));
    }

    if (operation->b_is_expr_value) {
        value_b = operation->value_b;
    } else {
        value_b = expr_solve_operation((operation_t*)(operation->value_b));
    }

    if (operation->operation == operation_sum) {
        if (
        value_a->type == String && 
        value_b->type == String) {
            result = malloc(strlen(value_a->value)+strlen(value_b->value)+1);
            strcpy(result, value_a->value);
            strcat(result, value_b->value);
            result_type = String;
        }
        else if (
        value_a->type == Number &&
        value_b->type == Number) {
            result = malloc(sizeof(double));
            *((double*)result) = (*(double*)value_a->value) + (*(double*)value_b->value);
            result_type = Number;
        }
        else if (
        value_a->type == String &&
        value_b->type == Number) {
            char* num_as_str = Number_toString(*(double*)value_b->value);
            result = malloc(strlen(value_a->value)+strlen(num_as_str)+1);
            strcpy(result, value_a->value);
            strcat(result, num_as_str);
            result_type = String;

            free(num_as_str);
        }
        else if (
        value_a->type == Number &&
        value_b->type == String) {
            char* num_as_str = Number_toString(*(double*)value_a->value);
            result = malloc(strlen(num_as_str)+strlen(value_b->value)+1);
            strcpy(result, num_as_str);
            strcat(result, value_b->value);
            result_type = String;

            free(num_as_str);
        }
    }

    return expr_value_create(result, result_type);

}

variable_t* expr_eval_from_compiled(List* compiled_expr, context_t* context) {
    
}