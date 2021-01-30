#ifndef LEXER_H
#define LEXER_H

#include "../libs/list/list.h"
#include <stdbool.h>

typedef enum {
    var_definition,
    var_type_specification,
    var_type_def_symbol,
    var_name,

    number,
    string_literal,

    array_begining,
    array_end,

    value_assignment,

    // Math operations:

    operation_sum,
    operation_sub,
    operation_mul,
    operation_div,

    // Condition

    cond_equal,
    cond_not_equal,
    cond_higher,
    cond_higher_or_equal,
    cond_less,
    cond_less_or_equal,

    // Function
    function_definition,
    function_name_definition,
    function_call,

    // Symbols
    comma,
    dot,
    semicolon,

    open_parenthesis,
    close_parenthesis,

    open_bracket,
    close_bracket,

    UNKNOWN

} token_type_t;

typedef struct {
    token_type_t type;
    char* token;
} token_t;

/*
    Lex the line and return a list with lexed tokens.
    @param line: The line to be lexed.
*/

bool lexer_is_number(char* str);
List* lexer_prelex_line(char* line);
List* lexer_define_tokens(List* prelexed_tokens);
List* lexer_lex_line(char* line);

extern char* token_type_str[];

#endif