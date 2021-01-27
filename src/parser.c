#include <string.h>
#include "parser.h"
#include "../libs/list/list.h"
#include "exceptions.h"
#include "errors.h"
#include "lexer.h"
#include "context.h"
#include "variable.h"

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

void parser_parse(char *line, context_t *context)
{
    List *tokenList = lexer_lex_line(line);

    for (int i = 0; i < tokenList->usedLength; i++)
    {
        token_t *token = list_get(tokenList, i);
        printf(" [\"%s\":%s]\n", token->token, token_type_str[token->type]);
    }
    printf("\n");

    parser_parse_tokens(tokenList, context);
}

int parser_internal_assign_variable(List *tokens, context_t *context, int i, char* varname) {

    token_t *token = list_get(tokens, i);

    variable_t *var = context_search_variable(context, varname);
    variable_type_t vartype = var->type;;

    // Next token should be a = symbol
    NEXT_TOKEN();
    if (token->type != value_assignment)
        ERR(SYNTAX_ERROR, "Assignment symbol expected (=)");

    char* valuemem;

    // Next should be a literal value, or a var name
    NEXT_TOKEN();
    if (token->type == string_literal)
    {
        if (vartype != String)
            ERR(TYPE_ERROR, "Wrong value assignment (string expected)");

        valuemem = malloc(strlen(token->token) + 1);
        strcpy(valuemem, token->token);
    }
    else if (token->type == var_name) {
        variable_t* otherVar = context_search_variable(context, token->token);
        if (var->type != Any && otherVar->type != Any
        && otherVar->type != var->type) {
            ERR(TYPE_ERROR, "Types are different");
        }
        if (otherVar->type != Object && otherVar->type != Array)
        {
            valuemem = malloc(otherVar->value_length);
            memcpy(valuemem, otherVar->value_pointer, otherVar->value_length);
        }
        else {
            valuemem = otherVar;
            var->is_pointer = true;
        }
    }

    if (var->value_pointer != NULL) variable_free_value(var);
    variable_assign(var, valuemem, strlen(valuemem) + 1);

    return i;
}

int parser_internal_create_variable(List *tokens, context_t *context, int i)
{
    token_t *token;
    // Next should be a name
    NEXT_TOKEN();
    if (token->type != var_name)
        ERR(SYNTAX_ERROR, "Name expected");
    char *varname = token->token;
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
        i = parser_internal_assign_variable(tokens, context, i, varname);
    
    // TODO: Implement other types
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
        else {
            ERR(SYNTAX_ERROR, "Expression required");
        }
    }
}

#undef NEXT_TOKEN
#undef GET_PREV_TOKEN