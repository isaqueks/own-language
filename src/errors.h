#ifndef ERRORS_H
#define ERRORS_H

#define TYPE_ERROR "Type error"
#define SYNTAX_ERROR "Syntax error"
#define MEMORY_ERROR "Memory error"
#define TODO_ERROR "Todo error"

void __RUNTIME_ERR(char* x, char* y, char* filename, int line);

void __ERR(char* x, char* y, char* token_type, char* token, int i, char* filename, int line);

#define RUNTIME_ERR(x, y) __RUNTIME_ERR(x, y, __FILE__, __LINE__)
#define ERR(x, y) __ERR(x, y, token_type_str[token->type], token->token, i, __FILE__, __LINE__)

#endif