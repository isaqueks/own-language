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
#include "expr.h"
#include "operations.h"

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

void* parser_eval_expr_item(expr_item_t* item, int* out_size, variable_type_t* type) {
    if (item->type == item_expr) {
        return parser_eval_compiled_expr(item->ptr, out_size, type);
    }
    else if (item->type == item_value) {
        // For now, let's consider just double
        // TODO: Implement arithmetitc for other types

        *type = item->value_type;
        void* mem;

        switch (item->value_type)
        {
        case Number:
            *out_size = sizeof(double);
            break;

        case String:
            *out_size = strlen(item->ptr)+1;
            break;
        
        default:
            Throw("Type not implemented");
            break;
        }

        mem = malloc(*out_size);
        memcpy(mem, item->ptr, *out_size);
        return mem;
    }
    else {
        RUNTIME_ERR(TYPE_ERROR, "Could not evaluate non value expr type!");
    }
}

void* parser_eval_compiled_expr(List* tree, int* out_size, variable_type_t* out_type) {

    // *out_type = Number;

    expr_item_t *op = NULL, *b = NULL;

    void* mem = parser_eval_expr_item(list_get(tree, 0), out_size, out_type);

    for (int j = 1; j < tree->used_length; j++) {
        expr_item_t* item = (expr_item_t*)list_get(tree, j);

        if (op == NULL) {
            op = item;
        }
        else if (b == NULL) {
            b = item;

            if (op->type != item_operator) {
                RUNTIME_ERR(TYPE_ERROR, "Term should be an operator!");
            }
            else if (b->type == item_operator) {
                RUNTIME_ERR(TYPE_ERROR, "Term should not be an operator!");
            }

            // Let's consider just sum for now
            
            variable_type_t b_type;
            int b_size;

            void* b_value = parser_eval_expr_item(b, &b_size, &b_type);

            // b = b_value,
            // a = mem

            void* a = mem;

            if (*out_type == String || b_type == String) {
                mem = &mem;
            }

            operation_calculate(
                (*(token_type_t*)op->ptr), 
                *out_type, 
                b_type, 
                a, 
                b_value,
                out_type,
                mem
            );


            op = NULL;
            b = NULL;
        }

    }
    
    return mem;
}

int parser_eval_expr(List *state, List *tokens, context_t *context,
                     variable_type_t type, int i, void **out, int* size,
                     variable_type_t *out_type
) {
    
    // First: let's search for the limit

    int start_i = i;
    int opened_parenthesis = 0;

    for (; i < tokens->used_length; i++) {
        token_t* token = (token_t*)list_get(tokens, i);
        
        if (token->type == open_parenthesis)  {
            opened_parenthesis++;
        }
        else if (token->type == close_parenthesis) {
            opened_parenthesis--;
            if (opened_parenthesis == 0) {
                break;
            }
        }
        else if (token->type == comma && opened_parenthesis == 0) {
            break;
        }
        else if (token->type == line_end && opened_parenthesis == 0) {
            break;
        }
    }    

    int out_i = start_i;
    List* expr = expr_parse_linear(tokens, context, &out_i, 0, i-1);
    List* tree = expr_mount_tree(expr, MAX_OP_PRIORITY);

    *out = parser_eval_compiled_expr(tree, size, out_type);

    return i;
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

    void* valuemem;
    int size;

    variable_type_t outtype;
    i = parser_eval_expr(state, tokens, context, var->type, i, &valuemem, &size,
                         &outtype);
    if (var->type == Any) {
        var->type = outtype;
    }
    else if (var->type != outtype) {
        RUNTIME_ERR(TYPE_ERROR, "Cannot change variable type!");    
    }
    
    if (var->value_pointer != NULL) { 
        variable_free_value(var);
    }
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
        i = parser_eval_expr(
            state, tokens, context,
            model->type, i, &value, &value_size,
            &expr_type
        );

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
    { 
        func = function_create_empty(fname, arglist);
    }

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
                      int *out_opened_brackets) {
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

        // printf(" [\"%s\":%s] \n", token->token, token_type_str[token->type]);

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