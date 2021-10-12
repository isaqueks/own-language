#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include "exceptions.h"
//#include "debug.h"
#include "lexer.h"
#include "errors.h"

extern char *token_type_str[] = {

    "var_definition",
    "var_type_specification",
    "var_type_def_symbol",
    "var_name",
    "number",
    "string_literal",
    "array_begining",
    "array_end",
    "value_assignment",
    "operation_sum",
    "operation_sub",
    "operation_mul",
    "operation_div",
    "operation_negation",
    "cond_equal",
    "cond_not_equal",
    "cond_higher",
    "cond_higher_or_equal",
    "cond_less",
    "cond_less_or_equal",
    "function_definition",
    "function_name_definition",
    "function_call",
    "comma",
    "dot",
    "line_end",
    "open_parenthesis",
    "close_parenthesis",
    "open_bracket",
    "close_bracket",
    // Keywords
    "if_keyword",
    "else_keyword",
    "while_keyword",

    "UNKNOWN"
    
};

// var x: String = "Hello World"

/*
    * var x: String = "Hello World" ==>
    ! var_definition {var}, var_name {x}, var_type_specification {:}
    ! value_assignment {=}, string_literal {Hello World}
*/

char Symbols[] = "=:.,()[]+-*/{};\n><!";
token_type_t SymbolTypes[] = {UNKNOWN, var_type_def_symbol, dot, comma, open_parenthesis,
    close_parenthesis, array_begining, array_end,

    operation_sum,
    operation_sub,
    operation_mul,
    operation_div,

    open_bracket,
    close_bracket,

    line_end,
    line_end,

    cond_higher,
    cond_less,

    operation_negation
};

char BeforeEqualComparsion[] = "=!><";
token_type_t BeforeEqualComparsionTypes[] = {
    cond_equal,
    cond_not_equal,
    cond_higher_or_equal,
    cond_less_or_equal
};

char* legal_name_start = "$_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

bool lexer_is_number(char* str) {
    char* start = str;
    while(*str) {
        if (!isdigit(*str) && !(*str == '.' && (str-start)>0))
            return false;
        *str++;
    }
    return true;
}

bool _isspace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r';
}

List *lexer_prelex_line(char *line)
{

    List *result = create_list(sizeof(token_t), 8);

    int index = 0;
    int length = strlen(line);

    bool insideString = false;

    char currentToken[128];
    int currentTokenIndex = 0;
    bool addNext = false;

    for (index = 0; index <= length; index++)
    {
        char ch = line[index];

        int symbolIndex = (index < length && ch != 0) ? strchr(Symbols, ch) : NULL;

        if (ch == '.' && symbolIndex != NULL) {
            currentToken[currentTokenIndex] = 0;
            if (lexer_is_number(currentToken))
            symbolIndex = NULL;
        }

        if (!insideString && ((currentTokenIndex == 0 && symbolIndex != NULL))) {
            addNext = true;
            currentToken[currentTokenIndex++] = ch;
            if (strchr(BeforeEqualComparsion, ch) != NULL && line[index+1] == '=') {
                currentToken[currentTokenIndex++] = line[++index];
            }
            ch = 0;
        }

        if (((
                _isspace(ch) ||
                symbolIndex != NULL ||
                ch == '"' 
                ) &&
             !insideString) ||
            (ch == '"' && insideString) ||
            addNext || index == length)
        {
            top: ;
            addNext = false;

            bool strLiteral = false;

            if (ch == '"')
            {
                if (insideString)
                    strLiteral = true;
                insideString = !insideString;
            }


            if (currentTokenIndex > 0)
            {
                currentToken[currentTokenIndex++] = 0x00;

                char *tok = malloc(currentTokenIndex);
                strcpy(tok, currentToken);

                token_t token;

                token.token = tok;
                token.type = UNKNOWN;

                if (strLiteral) {
                    token.type = string_literal;
                }
                else if (isdigit(currentToken[0]))
                {
                    bool isnum = lexer_is_number(currentToken);
                    if (isnum)
                        token.type = number;
                }

                list_add(result, &token);

                memset(currentToken, 0x00, 128);
                currentTokenIndex = 0;

                if (symbolIndex != NULL && ch > 0)
                {
                    currentToken[currentTokenIndex++] = ch;

                    addNext = true;

                    if (strchr(BeforeEqualComparsion, ch) != NULL && line[index+1] == '=' && currentTokenIndex == 1) {
                        currentToken[currentTokenIndex++] = line[++index];
                    }

                    ch = 0;
                    goto top;
                }
            }
            continue;
        }
        currentToken[currentTokenIndex++] = ch;
    }

    return result;
}

    #define ERR(e, x)                                      \
    {                                                      \
        printf(e ": " x " Received %s (\"%s\").",          \
               token_type_str[token->type], token->token); \
        return NULL;                                       \
    }

List *lexer_define_tokens(List *prelexed_tokens)
{

#define NEXT_TOKEN()                                    \
    {                                                   \
        if (i + 1 == prelexed_tokens->used_length)       \
            ERR(SYNTAX_ERROR, "Expression required.");  \
        token = list_get(prelexed_tokens, ++i); \
    }

#define GET_PREV_TOKEN() ((token_t *)list_get(prelexed_tokens, i-1))

    for (int i = 0; i < prelexed_tokens->used_length; i++)
    {
        token_t *token = (token_t *)list_get(prelexed_tokens, i);

        if (token->type == UNKNOWN && strcmp(token->token, "var") == 0)
        {
            token->type = var_definition;
            // Next token should be a name
            NEXT_TOKEN();
            if (token->type == UNKNOWN)
            {
                token->type = var_name;
                //Next should be a : symbol
            }
            else
            {
                ERR(SYNTAX_ERROR, "Variable name expected.");
            }
        }

        else if (token->type == UNKNOWN && strcmp(token->token, ":") == 0)
        {
            token->type = var_type_def_symbol;
            // Next token should be a type
            NEXT_TOKEN();
            if (token->type == UNKNOWN)
            {
                token->type = var_type_specification;
            }
            else
            {
                ERR(SYNTAX_ERROR, "Type expected!");
            }
        }

        else if (token->type == UNKNOWN && token->token[1] == '=' && 
        strlen(token->token) == 2 && strchr(BeforeEqualComparsion, token->token[0]) != NULL) {
            token_type_t condType = BeforeEqualComparsionTypes[strchr(BeforeEqualComparsion, token->token[0]) - BeforeEqualComparsion];
            token->type = condType;
        }

        else if (token->type == UNKNOWN && strcmp(token->token, "=") == 0) {
            token->type = value_assignment;
        }

        else if (token->type == UNKNOWN && strcmp(token->token, "function") == 0)
        {
            token->type = function_definition;
            // Next token should be a function name
            NEXT_TOKEN();
            if (token->type == UNKNOWN)
            {
                token->type = function_name_definition;
            }
        }

        else if (token->type == UNKNOWN && strcmp(token->token, "if") == 0)
        {
            token->type = if_keyword;
        }
        else if (token->type == UNKNOWN && strcmp(token->token, "else") == 0)
        {
            token->type = else_keyword;
        }
        else if (token->type == UNKNOWN && strcmp(token->token, "while") == 0)
        {
            token->type = while_keyword;
        }
        
        else if (token->type == UNKNOWN) {
            int symb = strlen(token->token)==1 ? strchr(Symbols, *token->token) : NULL;
            if (symb != NULL) {
                int index = symb - (int)((char*)Symbols);
                token_type_t stype = SymbolTypes[index];
                token->type = stype;
                if (stype == open_parenthesis &&
                i > 0 &&
                GET_PREV_TOKEN()->type == var_name &&
                strchr(legal_name_start, GET_PREV_TOKEN()->token[0]) != NULL)
                {
                    GET_PREV_TOKEN()->type = function_call;
                }
            }
            else if (strchr(legal_name_start, token->token[0]) != NULL) {
                token->type = var_name;
            }
        }
    }

    #undef NEXT_TOKEN
    return prelexed_tokens;
}

List *lexer_lex_line(char *line)
{
    List *tokenList = lexer_prelex_line(line);
    return lexer_define_tokens(tokenList);
}

#undef ERR