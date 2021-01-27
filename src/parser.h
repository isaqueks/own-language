#ifndef PARSER_H
#define PARSER_H

#include "../libs/list/list.h"
#include "exceptions.h"
#include "lexer.h"
#include "context.h"
#include "variable.h"

void parser_parse(char* line, context_t* context);
void parser_parse_tokens(List* tokens, context_t* context);

#endif