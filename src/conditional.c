
#include "conditional.h"

conditional_statement_t* conditional_statement_create
(conditional_type_t type) {
    conditional_statement_t* cond = malloc(sizeof(conditional_statement_t));

    cond->type = type;
    cond->condition_tokens  = create_list(sizeof(token_t), 8);
    cond->tokens        = create_list(sizeof(token_t), 8);

    return cond;
}

void conditional_statement_add_condition_token
(conditional_statement_t* cond, token_t* token) {
    list_add(cond->condition_tokens, token);
}

void conditional_statement_add_code
(conditional_statement_t* cond, token_t* token) {
    list_add(cond->tokens, token);
}

void conditional_statement_free(conditional_statement_t* cond) {
    for (int i = 0; i < cond->tokens; i++) {
        token_t* tok = ((token_t*)list_get(cond->tokens, i));
        free(tok->token);
    }
    list_free(cond->tokens);

    for (int i = 0; i < cond->condition_tokens; i++) {
        token_t* tok = ((token_t*)list_get(cond->tokens, i));
        free(tok->token);
    }
    list_free(cond->condition_tokens);

    free(cond);
}