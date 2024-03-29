#ifndef CONDITIONAL_H
#define CONDITIONAL_h

#include "../libs/list/list.h"
#include "context.h"
#include "lexer.h"

typedef enum {
    If,
    Else,
    While
} conditional_type_t;

typedef struct {

    List* condition_tokens;
    List* tokens;
    conditional_type_t type;
    context_t* scope;

} conditional_statement_t;


conditional_statement_t* conditional_statement_create(conditional_type_t type);
void conditional_statement_add_condition_token(conditional_statement_t* cond, token_t* token);
void conditional_statement_add_code(conditional_statement_t* cond, token_t* token);
void conditional_statement_create_scope(conditional_statement_t* cond, context_t* parent_scope);
void conditional_statement_set_scope(conditional_statement_t* cond, context_t* scope);
void conditional_statement_free(conditional_statement_t* cond);

#endif