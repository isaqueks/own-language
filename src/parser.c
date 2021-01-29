#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "../libs/list/list.h"
#include "exceptions.h"
#include "errors.h"
#include "lexer.h"
#include "context.h"
#include "variable.h"
#include "primtypes.h"

#define NEXT_TOKEN()                                   \
    {                                                  \
        if (i + 1 == tokens->usedLength)               \
            ERR(SYNTAX_ERROR, "Expression required."); \
        token = list_get(tokens, ++i);                 \
    }

#define GET_PREV_TOKEN() ((token_t *)list_get(tokens, i - 1))
#define GET_NEXT_TOKEN() ((token_t *)list_get(tokens, i +1))

#define ERR(x, y)                                                                                                             \
    {                                                                                                                         \
        printf("[" __FILE__ ":%d] " x ": " y ": %s (\"%s\") received.", __LINE__, token_type_str[token->type], token->token); \
        while (1)                                                                                                             \
            ;                                                                                                                 \
    }

int parser_eval_expr_until_tokens(List* tokens, context_t* context,
variable_type_t type, int i, token_type_t limits[], int limitscount, void** out, int* size) {
    bool isString = false;

    void* output = NULL;
    int outSize = 0;

    bool operation_time = false;
    token_type_t operation;

    for (; i < tokens->usedLength; i++) {
        token_t* token = list_get(tokens, i);
        bool brk = false;
        for (int j = 0; j < limitscount; j++) {
            if (token->type == limits[j]
            && token->type != UNKNOWN)
            {
                brk = true;
                break;
            }
        }
        if(brk) break;

        variable_type_t token_type_as_var_type;

        bool shouldContinue = false;
        switch (token->type)
        {
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

        char* value = token->token;
        // A number can be represented in string
        bool value_is_string_representation = true;

        if (token_type_as_var_type == -1) {
            variable_t* var = context_search_variable(context, token->token);
            token_type_as_var_type = var->type;
            value = (char*)(variable_get_value(var));
            if (token_type_as_var_type != String)
                value_is_string_representation = false;
            
        }

        if (token_type_as_var_type != type)
            ERR(TYPE_ERROR, "Type conflict.");

        if (output == NULL) {
            switch (type)
            {
            case Number:
                outSize = sizeof(double);
                break;

            case String:
                outSize = strlen(value)+1;
                break;

            default:
                Throw("NOT_IMPLEMENTED");
                break;
            }
            output = malloc(outSize);
        }
        else {
            switch (type)
            {
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
            
            double* out_as_double_ptr = ((double*)output);
            #define STR2NUM(x) (value_is_string_representation ? String_toNumber(x) : *out_as_double_ptr); \

            if (operation == operation_sum) {
                if (type == String) {
                    outSize += strlen(value);
                    strcat((char*)output, value);
                }
                else if (type == Number) {
                    *out_as_double_ptr += STR2NUM(value);
                }
            }
            if (operation == operation_sub) {
                if (type == String) {
                    ERR(TYPE_ERROR, "Cannot subtract strings!")
                }
                else if (type == Number) {
                    *out_as_double_ptr -= STR2NUM(value);
                }
            }
            if (operation == operation_mul) {
                if (type == String) {
                    ERR(TYPE_ERROR, "Cannot multiply strings!")
                }
                else if (type == Number) {
                    *out_as_double_ptr *= STR2NUM(value);
                }
            }
            if (operation == operation_div) {
                if (type == String) {
                    ERR(TYPE_ERROR, "Cannot divide strings!")
                }
                else if (type == Number) {
                    *out_as_double_ptr /= STR2NUM(value);
                }
            }

            operation_time = false;
            continue;
        }
        else {

            // This code will run on firt iteration
            // It will initialize output
            if (type == Number) {
                double real_number_value = 0;

                if (value_is_string_representation)
                    real_number_value = String_toNumber(value);
                else    
                    real_number_value = *((double*)value);

                double* dptr = (double*)output;
                *dptr = real_number_value; 

            }
            else if (type == String) {
                strcpy((char*)output, value);
            }
        }
        #undef STR2NUM
    }
    
    *size = outSize;
    *out = output;
    return i;
}

int parser_eval_expr(List* tokens, context_t* context, variable_type_t type, int i, void** out, int* size)
{
    token_type_t limits[] = { UNKNOWN };
    return parser_eval_expr_until_tokens(tokens, context, type, i, limits, 1, out, size);
}

void parser_parse(char *line, context_t *context)
{
    List *tokenList = lexer_lex_line(line);

    // for (int i = 0; i < tokenList->usedLength; i++)
    // {
    //     token_t *token = list_get(tokenList, i);
    //     printf(" [\"%s\":%s]\n", token->token, token_type_str[token->type]);
    // }
    // printf("\n");

    parser_parse_tokens(tokenList, context);

    for (int i = 0; i < tokenList->usedLength; i++) {
        token_t* token = (token_t*)list_get(tokenList, i);
        free(token->token);
    }

    free(tokenList->objArray);
    free(tokenList);
}

int parser_internal_assign_variable(List *tokens, context_t *context, int i, char* varname) {

    token_t *token = list_get(tokens, i);

    variable_t *var = context_search_variable(context, varname);
    variable_type_t vartype = var->type;;

    // Next token should be a = symbol
    NEXT_TOKEN();
    if (token->type != value_assignment)
        ERR(SYNTAX_ERROR, "Assignment symbol expected (=)");


    // Next should be a literal value, or a var name
    NEXT_TOKEN();
    char* valuemem;


    // if (token->type == string_literal)
    // {
    //     if (vartype != String)
    //         ERR(TYPE_ERROR, "Wrong value assignment (string expected)");

    //     valuemem = malloc(strlen(token->token) + 1);
    //     strcpy(valuemem, token->token);
    // }
    // else if (token->type == number) {
    //     double value = 0;
    //     value = strtod(token->token, NULL);
    //     if (value == 0) {
    //         if (errno == ERANGE)
    //             ERR(MEMORY_ERROR, "Out of range in double conversion.");
    //     }

    //     valuemem = malloc(sizeof(double));
    //     memcpy(valuemem, &value, sizeof(double));
    // }
    // else if (token->type == var_name) {
    //     variable_t* otherVar = context_search_variable(context, token->token);
    //     if (var->type != Any && otherVar->type != Any
    //     && otherVar->type != var->type) {
    //         ERR(TYPE_ERROR, "Types are different");
    //     }
    //     if (otherVar->type != Object && otherVar->type != Array)
    //     {
    //         valuemem = malloc(otherVar->value_length);
    //         memcpy(valuemem, otherVar->value_pointer, otherVar->value_length);
    //     }
    //     else {
    //         valuemem = otherVar;
    //         var->is_pointer = true;
    //     }
    // }

    int size;
    i = parser_eval_expr(tokens, context, var->type, i, &valuemem, &size);

    if (var->value_pointer != NULL) variable_free_value(var);

    variable_assign(var, valuemem, size);

    return i;
}

int parser_internal_create_variable(List *tokens, context_t *context, int i)
{
    token_t *token;
    // Next should be a name
    NEXT_TOKEN();
    if (token->type != var_name)
        ERR(SYNTAX_ERROR, "Name expected");

    char *varname = malloc(strlen(token->token)+1);
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

    // TODO: Implement Array: T (var x: Array: String = ["Hello", "World"])
    // if (vartype == Array) {
    //     if (GET_NEXT_TOKEN()->type == var_type_def_symbol) {

    //     }
    //     else {
    //         ERR(SYNTAX_ERROR, "Type expected. Array must have a sub-type! Use Array: T.");
    //     }
    // }

    if (GET_NEXT_TOKEN()->type == value_assignment)
        i = parser_internal_assign_variable(tokens, context, i, varname);
    
    // TODO: Implement other types
    return i;
}

void parser_function_invoke(function_t* function, context_t* context) {
    if (function->inner_code == 0) // Native
        function->native_function(context);
    else // TODO: Implement non native functions
        Throw("Not implemented!");
}

int parser_call_function(List* tokens, context_t* context, int i) {

    token_t* token = list_get(tokens, i);
    if (token->type != function_call)
        ERR(SYNTAX_ERROR, "Function call expected!");

    char* fname = token->token;
    function_t* function = context_search_function(context, fname);

    NEXT_TOKEN(); // Should be a (

    if (token->type != open_parenthesis)
        ERR(SYNTAX_ERROR, "Opening parenthesis expected (()!");
    
    context_t* function_scope = context_create(context);

    int model_count=0;
    for (i++; i < tokens->usedLength; i++) {

        token = list_get(tokens, i);
        if (token->type == close_parenthesis)
            break;

        variable_t* model = *(variable_t**)list_get(function->arg_models, model_count++);

        int value_size = 0;
        void* value = NULL;

        char* vname = malloc(strlen(model->name)+1);
        strcpy(vname, model->name);

        // if (token->type == string_literal) {
        //     value_size = strlen(token->token)+1;
        //     value = malloc(value_size);
        //     strcpy(value, token->token);
        // }
        // else if (token->type == number) {
        //     value_size = sizeof(double);
        //     value = malloc(value_size);
        //     double* value_as_double = ((double*)value);
        //     *value_as_double = String_toNumber(token->token);
        // }
        // else if (token->type == var_name) {
        //     variable_t* var = context_search_variable(context, token->token);
        //     if (var->type != model->type)
        //         ERR(TYPE_ERROR, "Type conflict");
        //     void* var_value = variable_get_value(var);
        //     value_size = var->value_length;
        //     value = malloc(value_size);
        //     memcpy(value, var_value, value_size);
        // }

        token_type_t toks[] = {comma, close_parenthesis};
        i = parser_eval_expr_until_tokens(tokens, context, model->type, i,
        toks, 2, &value, &value_size);

        variable_t* arg = variable_create(vname, model->type, value, value_size);
        context_add_variable(function_scope, arg);

    }

    parser_function_invoke(function, function_scope);

    context_free(function_scope);

    return i;
}

void parser_parse_tokens(List *tokens, context_t *context)
{
    for (int i = 0; i < tokens->usedLength; i++)
    {
        token_t *token = list_get(tokens, i);

        if (token->type == var_definition) {
            i = parser_internal_create_variable(tokens, context, i);
        }
        else if (token->type == var_name) {
            // Should be an assignment
            i = parser_internal_assign_variable(tokens, context, i, token->token);
        }
        else if (token->type == function_call) {
            i = parser_call_function(tokens, context, i);
        }
        else {
            ERR(SYNTAX_ERROR, "Expression required");
        }
    }
}

#undef NEXT_TOKEN
#undef GET_PREV_TOKEN