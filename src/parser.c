#include "parser.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../libs/list/list.h"
#include "conditional.h"
#include "context.h"
#include "errors.h"
#include "exceptions.h"
#include "expr.h"
#include "lexer.h"
#include "primtypes.h"
#include "variable.h"

#define NEXT_TOKEN()                                   \
    {                                                  \
        if (i + 1 == tokens->used_length)              \
            ERR(SYNTAX_ERROR, "Expression required."); \
        token = list_get(tokens, ++i);                 \
    }

#define BREAK_IF_LINE_END() \
    if (token->type == line_end) break;

#define GET_PREV_TOKEN() ((token_t *)list_get(tokens, i - 1))
#define GET_NEXT_TOKEN() ((token_t *)list_get(tokens, i + 1))

#define GET_TOP_STATE()      \
    (state->used_length == 0 \
         ? NULL              \
         : (parser_state_t *)(list_get(state, state->used_length - 1)))

bool parser_set_state(List *state, parser_state_task_t current_tasks[],
                      int taskc, parser_state_t *state_to_add) {
    parser_state_t *curr = GET_TOP_STATE();
    if (curr == NULL) {
        list_add(state, state_to_add);
        return true;
    } else {
        int equal_count = 0;
        for (int i = 0; i < taskc; i++) {
            if (current_tasks[i] == curr->task) {
                equal_count++;
            }
        }
        if (equal_count > 0) {
            list_set(state, state->used_length - 1, state_to_add);
            return false;
        } else {
            list_add(state, state_to_add);
            return true;
        }
    }
}

bool parser_remove_state_if_mine(List *state,
                                 parser_state_task_t current_tasks[],
                                 int taskc) {
    parser_state_t *curr = GET_TOP_STATE();
    if (curr == NULL) {
        return false;
    } else {
        int equal_count = 0;
        for (int i = 0; i < taskc; i++) {
            if (current_tasks[i] == curr->task) {
                equal_count++;
            }
        }
        if (equal_count == 0) {
            return false;
        } else {
            list_remove_at(state, state->used_length - 1);
            return true;
        }
    }
}

int parser_eval_expr_until_tokens(List *state, List *tokens, context_t *context,
                                  variable_type_t type, int i,
                                  token_type_t limits[], int limitscount,
                                  void **out, int *size,
                                  variable_type_t *out_type) {
    bool isString = false;

    void *output = NULL;
    int outSize = 0;

    bool operation_time = false;
    token_type_t operation;

    for (; i < tokens->used_length; i++) {
        token_t *token = list_get(tokens, i);
        BREAK_IF_LINE_END();

        bool brk = false;
        for (int j = 0; j < limitscount; j++) {
            if (token->type == limits[j] && token->type != UNKNOWN) {
                brk = true;
                break;
            }
        }
        if (brk) break;

        variable_type_t token_type_as_var_type;

        bool shouldContinue = false;
        switch (token->type) {
            case operation_sum:
                operation = operation_sum;
                operation_time = true;
                shouldContinue = true;
                break;
            case operation_sub:
                operation = operation_sub;
                operation_time = true;
                shouldContinue = true;
                break;
            case operation_mul:
                operation = operation_mul;
                operation_time = true;
                shouldContinue = true;
                break;
            case operation_div:
                operation = operation_div;
                operation_time = true;
                shouldContinue = true;
                break;
            case cond_equal:
                operation = cond_equal;
                operation_time = true;
                shouldContinue = true;
                break;
            case cond_higher:
                operation = cond_higher;
                operation_time = true;
                shouldContinue = true;
                break;
            case cond_higher_or_equal:
                operation = cond_higher_or_equal;
                operation_time = true;
                shouldContinue = true;
                break;
            case cond_less:
                operation = cond_less;
                operation_time = true;
                shouldContinue = true;
                break;
            case cond_less_or_equal:
                operation = cond_less_or_equal;
                operation_time = true;
                shouldContinue = true;
                break;
            case cond_not_equal:
                operation = cond_not_equal;
                operation_time = true;
                shouldContinue = true;
                break;
            case number:
                token_type_as_var_type = Number;
                break;
            case string_literal:
                token_type_as_var_type = String;
                break;
            case var_name:
                token_type_as_var_type = -1;
                break;
            // TODO: Add other types
            default:
                ERR(SYNTAX_ERROR, "Constant value or variable expected.");
                break;
        }
        if (shouldContinue) {
            shouldContinue = false;
            continue;
        }

        char *value = token->token;
        // A number can be represented in string
        bool value_is_string_representation = true;

        if (token_type_as_var_type == -1) {
            variable_t *var = context_search_variable(context, token->token);
            token_type_as_var_type = var->type;
            value = (char *)(variable_get_value(var));
            if (token_type_as_var_type != String)
                value_is_string_representation = false;
        }

        if (token_type_as_var_type != type && type != Any &&
            token_type_as_var_type != Any) {
            ERR(TYPE_ERROR, "Type conflict.");
        }

        if (output == NULL) {
            if (type == Any) {
                type = token_type_as_var_type;
            }

            switch (type) {
                case Number:
                    outSize = sizeof(double);
                    break;

                case String:
                    outSize = strlen(value) + 1;
                    break;

                default:
                    Throw("NOT_IMPLEMENTED");
                    break;
            }
            output = malloc(outSize);
        } else {
            switch (type) {
                case Number:
                    // Same size, no changes
                    outSize = sizeof(double);
                    break;

                case String:
                    outSize += strlen(value);
                    break;

                default:
                    Throw("NOT_IMPLEMENTED");
                    break;
            }
            output = realloc(output, outSize);
        }

        if (operation_time) {
            double *out_as_double_ptr = ((double *)output);
#define STR2NUM(x) \
    (value_is_string_representation ? String_toNumber(x) : *((double *)(x)))

            if (operation == operation_sum) {
                if (type == String) {
                    outSize += strlen(value);
                    strcat((char *)output, value);
                } else if (type == Number) {
                    double val = STR2NUM(value);
                    *out_as_double_ptr += STR2NUM(value);
                }
            }
            if (operation == operation_sub) {
                if (type == String) {
                    ERR(TYPE_ERROR, "Cannot subtract strings!");
                } else if (type == Number) {
                    *out_as_double_ptr -= STR2NUM(value);
                }
            }
            if (operation == operation_mul) {
                if (type == String) {
                    ERR(TYPE_ERROR, "Cannot multiply strings!");
                } else if (type == Number) {
                    *out_as_double_ptr *= STR2NUM(value);
                }
            }
            if (operation == operation_div) {
                if (type == String) {
                    ERR(TYPE_ERROR, "Cannot divide strings!");
                } else if (type == Number) {
                    *out_as_double_ptr /= STR2NUM(value);
                }
            }
            if (operation == cond_equal || operation == cond_not_equal ||
                operation == cond_higher || operation == cond_higher_or_equal ||
                operation == cond_less || operation == cond_less_or_equal) {
                int boolValue = false;
                if (type == String) {
                    switch (operation) {
                        case cond_equal:
                            boolValue = (strcmp(output, value) == 0);
                            break;
                        case cond_not_equal:
                            boolValue = (strcmp(output, value) != 0);
                            break;
                        case cond_higher:
                            boolValue = strlen(output) > strlen(value);
                            break;
                        case cond_higher_or_equal:
                            boolValue = strlen(output) >= strlen(value);
                            break;
                        case cond_less:
                            boolValue = strlen(output) < strlen(value);
                            break;
                        case cond_less_or_equal:
                            boolValue = strlen(output) <= strlen(value);
                            break;
                    }
                } else if (type == Number) {
                    switch (operation) {
                        case cond_equal:
                            boolValue = (*out_as_double_ptr == STR2NUM(value));
                            break;
                        case cond_not_equal:
                            boolValue = (*out_as_double_ptr != STR2NUM(value));
                            break;
                        case cond_higher:
                            boolValue = (*out_as_double_ptr > STR2NUM(value));
                            break;
                        case cond_higher_or_equal:
                            boolValue = (*out_as_double_ptr >= STR2NUM(value));
                            break;
                        case cond_less:
                            boolValue = (*out_as_double_ptr < STR2NUM(value));
                            break;
                        case cond_less_or_equal:
                            boolValue = (*out_as_double_ptr <= STR2NUM(value));
                            break;
                    }
                }
                outSize = sizeof(double);
                *out_as_double_ptr = boolValue;
                type = Number;
            }

            operation_time = false;
            continue;
        } else {
            // This code will run on firt iteration
            // It will initialize output
            if (type == Number) {
                double real_number_value = 0;

                if (value_is_string_representation)
                    real_number_value = String_toNumber(value);
                else
                    real_number_value = *((double *)value);

                double *dptr = (double *)output;
                *dptr = real_number_value;
            } else if (type == String) {
                strcpy((char *)output, value);
            }
        }
    }

    *out_type = type;
    *size = outSize;
    *out = output;
    return i;
#undef STR2NUM
}

int parser_eval_expr(List *state, List *tokens, context_t *context,
                     variable_type_t type, int i, void **out, int *size,
                     variable_type_t *out_type) {
    token_type_t limits[] = {UNKNOWN};
    return parser_eval_expr_until_tokens(state, tokens, context, type, i,
                                         limits, 1, out, size, out_type);
}

void parser_parse(List *state, char *line, context_t *context) {
    List *tokenList = lexer_lex_line(line);

    parser_parse_tokens(state, tokenList, context);

    for (int i = 0; i < tokenList->used_length; i++) {
        token_t *token = (token_t *)list_get(tokenList, i);
        free(token->token);
    }

    free(tokenList->objArray);
    free(tokenList);
}

int parser_internal_assign_variable(List *state, List *tokens,
                                    context_t *context, int i, char *varname) {
    token_t *token = list_get(tokens, i);

    variable_t *var = context_search_variable(context, varname);
    variable_type_t vartype = var->type;
    ;

    // Next token should be a = symbol
    NEXT_TOKEN();
    if (token->type != value_assignment)
        ERR(SYNTAX_ERROR, "Assignment symbol expected (=)");

    // Next should be a literal value, or a var name
    NEXT_TOKEN();

    char *valuemem;
    int size;

    variable_type_t outtype;
    i = parser_eval_expr(state, tokens, context, var->type, i, &valuemem, &size,
                         &outtype);
    if (var->type == Any) {
        var->type = outtype;
    }

    if (var->value_pointer != NULL) variable_free_value(var);
    variable_assign(var, valuemem, size);

    return i;
}

int parser_internal_create_variable(List *state, List *tokens,
                                    context_t *context, int i) {
    token_t *token;
    // Next should be a name
    NEXT_TOKEN();
    if (token->type != var_name) ERR(SYNTAX_ERROR, "Name expected");

    char *varname = malloc(strlen(token->token) + 1);
    strcpy(varname, token->token);
    // Next should be a type symb :
    NEXT_TOKEN();
    if (token->type != var_type_def_symbol)
        ERR(SYNTAX_ERROR, "Expected var definition symbol (:).");

    // Next should be a type specification
    NEXT_TOKEN();
    if (token->type != var_type_specification)
        ERR(SYNTAX_ERROR, "Type expected");

    char *vartype_str = token->token;
    variable_type_t vartype = variable_get_type_by_string(vartype_str);

    variable_t *var = variable_create(varname, vartype, NULL, 0);
    context_add_variable(context, var);

    if (GET_NEXT_TOKEN()->type == value_assignment)
        i = parser_internal_assign_variable(state, tokens, context, i, varname);

    return i;
}

void parser_function_invoke(List *state, function_t *function,
                            context_t *func_scope) {
    if (function->tokens == NULL)  // Native
        function->native_function(func_scope);
    else {
        parser_parse_tokens(state, function->tokens, func_scope);
    }
}

int parser_call_function(List *state, List *tokens, context_t *context, int i) {
    token_t *token = list_get(tokens, i);
    if (token->type != function_call)
        ERR(SYNTAX_ERROR, "Function call expected!");

    char *fname = token->token;
    function_t *function = context_search_function(context, fname);

    NEXT_TOKEN();  // Should be a (

    if (token->type != open_parenthesis)
        ERR(SYNTAX_ERROR, "Opening parenthesis expected (()!");

    context_t *function_scope = context_create(context);

    int model_index = 0;
    for (i++; i < tokens->used_length; i++) {
        token = list_get(tokens, i);
        BREAK_IF_LINE_END();

        if (token->type == close_parenthesis) break;

        if (model_index >= function->arg_models->used_length) {
            RUNTIME_ERR(SYNTAX_ERROR,
                        "More arguments than expected were passed.");
        }

        variable_t *model =
            *(variable_t **)list_get(function->arg_models, model_index++);

        int value_size = 0;
        void *value = NULL;

        char *vname = malloc(strlen(model->name) + 1);
        strcpy(vname, model->name);

        variable_type_t expr_type;
        token_type_t toks[] = {comma, close_parenthesis};
        i = parser_eval_expr_until_tokens(state, tokens, context, model->type,
                                          i, toks, 2, &value, &value_size,
                                          &expr_type);

        variable_t *arg = variable_create(vname, expr_type, value, value_size);
        context_add_variable(function_scope, arg);
    }

    parser_function_invoke(state, function, function_scope);

    context_free(function_scope);

    return i;
}

int parser_create_function(List *state, List *tokens, context_t *context,
                           int i) {
    parser_state_t *currState = GET_TOP_STATE();
    parser_state_task_t thistask[] = {func_creation_awaiting_end};
    bool executing_by_state =
        currState != NULL && (currState->task == func_creation_awaiting_end);

    token_t *token = list_get(tokens, i);

    function_t *func;

    if (executing_by_state) {
        func = (function_t *)currState->memory;
        goto function_submit;
    }

    if (token->type != function_definition) {
        ERR(SYNTAX_ERROR, "Function definition expected");
    }

    // Should be the function name
    NEXT_TOKEN();
    if (token->type != function_name_definition) {
        ERR(SYNTAX_ERROR, "Function name expected");
    }

    char *fname = malloc(strlen(token->token) + 1);
    strcpy(fname, token->token);

    // The next token should be a ( if it has arguments or a {
    // Tokens end reached
    // So, no-arg function and multiline
    if (i + 1 == tokens->used_length) {
        goto function_create;
    }
    List *arglist = create_list(sizeof(variable_t *), 8);
    NEXT_TOKEN();
    if (token->type == open_parenthesis) {
        for (++i; i < tokens->used_length; i++) {
            // arg: T (3 tokens per arg)
            token = list_get(tokens, i);
            if (token->type != var_name)
                ERR(SYNTAX_ERROR, "Variable name expected!");

            char *argname = malloc(strlen(token->token) + 1);
            strcpy(argname, token->token);

            NEXT_TOKEN();

            if (token->type != var_type_def_symbol)
                ERR(SYNTAX_ERROR, "Type definition symbol (:) expected");

            NEXT_TOKEN();

            if (token->type != var_type_specification)
                ERR(SYNTAX_ERROR, "Type expected!");

            variable_type_t type;
            if (strcmp(token->token, "String") == 0)
                type = String;
            else if (strcmp(token->token, "Number") == 0)
                type = Number;
            else if (strcmp(token->token, "Object") == 0)
                type = Object;
            else if (strcmp(token->token, "Array") == 0)
                type = Array;
            else if (strcmp(token->token, "Any") == 0)
                type = Any;
            else
                ERR(TYPE_ERROR,
                    "Invalid type! Must be String, Number, Object, Array or "
                    "Any!");

            variable_t *arg = variable_create(argname, type, NULL, 0);
            list_add(arglist, &arg);

            NEXT_TOKEN();
            if (token->type == comma) {
                continue;
            } else if (token->type == close_parenthesis) {
                if (i < tokens->used_length - 1) {
                    NEXT_TOKEN();
                }
                break;
            } else {
                ERR(SYNTAX_ERROR, "Closing parenthesis expected");
            }
        }
    }

function_create:;
    { func = function_create_empty(fname, arglist); }

function_read_code:;
    {
        parser_set_state(state, thistask, 1,
                         &STATE(func_creation_awaiting_end, 0, func));
        int output_status = 0;
        i = parser_read_block_save_state(state, tokens, context, i,
                                         func->tokens, &output_status);
        if (output_status < 0) {
            return i;
        }
    }

function_submit:;
    {
        parser_remove_state_if_mine(state, thistask, 1);
        context_add_function(context, func);
    }

    return i;
}

int parser_read_block(List *tokens, context_t *context, int i, List **out_block,
                      int **out_opened_brackets) {
    // List of tokens
    List *block = *out_block;

    token_t *token = (token_t *)list_get(tokens, i);

    bool executing_by_state = false;
    int opened_brackets = 1;

    if (block == NULL) {
        block = create_list(sizeof(token_t), 16);
    }

    if (*out_opened_brackets == -1) {
        executing_by_state = false;
    } else {
        executing_by_state = true;
        opened_brackets = *out_opened_brackets;
        goto read_tokens;
    }

    if (token->type != open_bracket) {
        ERR(SYNTAX_ERROR, "Openning brackets ({) expected! ");
    }

    NEXT_TOKEN();

read_tokens:;
    while (true) {
        if (token->type == open_bracket) {
            opened_brackets++;
        } else if (token->type == close_bracket) {
            opened_brackets--;
            if (opened_brackets == 0) {
                break;
            }
        }

        token_t clone;
        clone.type = token->type;
        clone.token = malloc(strlen(token->token) + 1);
        strcpy(clone.token, token->token);

        list_add(block, &clone);

        if (i >= tokens->used_length - 1) {
            break;
        }

        NEXT_TOKEN();
    }

    *out_opened_brackets = opened_brackets;
    return i;
}

int parser_read_block_save_state(List *state, List *tokens, context_t *context,
                                 int i, List *block, int *status) {
    token_t *token = list_get(tokens, i);
    int opened_brackets = -1;
    parser_state_t *last_state = GET_TOP_STATE();

    token_type_t read_block_states[] = {block_awaiting_start,
                                        block_awaiting_end};

    if (last_state != NULL) {
        if (last_state->task == block_awaiting_start) {
            opened_brackets = last_state->flag;
            block = last_state->memory;
            goto read_until_open_bracket;
        } else if (last_state->task == block_awaiting_end) {
            opened_brackets = last_state->flag;
            block = last_state->memory;
            goto read_until_end;
        }
    }

read_until_open_bracket:;
    {
        parser_set_state(state, read_block_states, 2,
                         &STATE(block_awaiting_start, opened_brackets, block));
        while (token->type == line_end) {
            if (i >= tokens->used_length - 1) {
                break;
            }
            NEXT_TOKEN();
        }

        if (token->type == line_end) {
            // Tokens can end and the "{" can be on a next line

            *status = -3;
            return i;

        } else if (token->type != open_bracket) {
            ERR(SYNTAX_ERROR, "Openning bracket ({) expected!");
        }

        parser_remove_state_if_mine(state, read_block_states, 2);

        if (i >= tokens->used_length - 1) {
            *status = -1;
            return i;
        }
    }

read_until_end:;
    {
        parser_set_state(state, read_block_states, 2,
                         &STATE(block_awaiting_end, opened_brackets, block));

        i = parser_read_block(tokens, context, i, &block, &opened_brackets);
        if (opened_brackets != 0) {
            parser_set_state(
                state, read_block_states, 2,
                &STATE(block_awaiting_end, opened_brackets, block));
            *status = -2;
            return i + 1;
        }

        parser_remove_state_if_mine(state, read_block_states, 2);
    }

    *status = 1;
    return i + 1;
}

int parser_create_conditional_statement(List *state, List *tokens,
                                        context_t *context, int i) {
    bool executing_by_state = false;
    token_t *token = (token_t *)list_get(tokens, i);
    conditional_statement_t *statement;
    parser_state_task_t thistasks[] = {conditional_awaiting_end};

    parser_state_t *last_state = GET_TOP_STATE();

    if (last_state != NULL && last_state->task == conditional_awaiting_end) {
        executing_by_state = true;
        statement = (conditional_statement_t *)last_state->memory;
        goto after_reading_block;
    }

    conditional_type_t cond_type;

    switch (token->type) {
        case if_keyword:
            cond_type = If;
            break;
        case while_keyword:
            cond_type = While;
            break;
        default:
            ERR(SYNTAX_ERROR, "Unknown statement type.");
            break;
    }

    statement = conditional_statement_create(cond_type);

    NEXT_TOKEN();

    if (token->type != open_parenthesis) {
        ERR(SYNTAX_ERROR, "Opening parenthesis expected!");
    }

    NEXT_TOKEN();

    while (token->type != close_parenthesis) {
        token_t token_clone;
        token_clone.type = token->type;
        token_clone.token = malloc(strlen(token->token) + 1);
        strcpy(token_clone.token, token->token);

        conditional_statement_add_condition_token(statement, &token_clone);
        NEXT_TOKEN();
    }

    // Check anyway, because loop can end due to seg fault
    if (token->type != close_parenthesis) {
        ERR(SYNTAX_ERROR, "Closing parenthesis expected.");
    }

    NEXT_TOKEN();

    // State to continue

read_after_open_bracket:;
    {
        parser_set_state(state, thistasks, 1,
                         &STATE(conditional_awaiting_end, 0, statement));

        int output_status = 0;
        i = parser_read_block_save_state(state, tokens, context, i,
                                         statement->tokens, &output_status);
        if (output_status < 0) {
            return i;
        }
    }

after_reading_block:;
    { parser_remove_state_if_mine(state, thistasks, 1); }

eval_condition:;
    int outputSize = 0;
    double *output;
    variable_type_t outputType;

    parser_eval_expr(state, statement->condition_tokens, context, Number, 0,
                     &output, &outputSize, &outputType);

    if (outputType != Number) {
        RUNTIME_ERR(TYPE_ERROR, "Condition should return a Number!");
    }

    if (*output > 0) {
        conditional_statement_create_scope(statement, context);
        parser_parse_tokens(state, statement->tokens, statement->scope);

        if (statement->type == While) {
            goto eval_condition;
        }

        conditional_statement_free(statement);
    }

    return i;
}

#define REQUIRE_TOKENS 1
#define SUCCESS 0

void parser_parse_tokens(List *state, List *tokens, context_t *context) {
    // * Token debug -- Not needed anymore
    // for (int i = 0; i < tokens->used_length; i++)
    // {
    //     token_t *token = list_get(tokens, i);
    //     printf(" [\"%s\":%s] \n", token->token, token_type_str[token->type]);
    // }
    // printf("\n");

    int i = 0;

    parser_state_t *curr_state = GET_TOP_STATE();
    if (curr_state != NULL) {
        int tmp = 0;
        switch (curr_state->task) {
            case func_creation_awaiting_end:
                i = parser_create_function(state, tokens, context, i);
                break;
            case conditional_awaiting_end:
                i = parser_create_conditional_statement(state, tokens, context,
                                                        i);
                break;
            case block_awaiting_start:
                i = parser_read_block_save_state(state, tokens, context, i,
                                                 NULL, &tmp);
                break;
            case block_awaiting_end:
                i = parser_read_block_save_state(state, tokens, context, i,
                                                 NULL, &tmp);
                break;
            default:
                Throw("Invalid state");
                break;
        }
    }

    for (; i < tokens->used_length; i++) {
        token_t *token = list_get(tokens, i);

        if (token->type == line_end) {
            continue;
        } else if (token->type == var_definition) {
            i = parser_internal_create_variable(state, tokens, context, i);
        } else if (token->type == var_name) {
            // Should be an assignment
            i = parser_internal_assign_variable(state, tokens, context, i,
                                                token->token);
        } else if (token->type == function_call) {
            i = parser_call_function(state, tokens, context, i);
        } else if (token->type == function_definition) {
            i = parser_create_function(state, tokens, context, i);
        }
        // If conditional
        else if (token->type == if_keyword) {
            i = parser_create_conditional_statement(state, tokens, context, i);
        } else if (token->type == while_keyword) {
            i = parser_create_conditional_statement(state, tokens, context, i);
        } else {
            ERR(SYNTAX_ERROR, "Expression required");
        }
    }
}

#undef NEXT_TOKEN
#undef GET_PREV_TOKEN