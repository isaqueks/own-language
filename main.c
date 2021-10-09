#include <stdio.h>
#include <stdlib.h>

#include "libs/list/list.h"

#include "src/exceptions.h"
#include "src/variable.h"
#include "src/lexer.h"
#include "src/context.h"
#include "src/parser.h"
#include "src/function.h"
#include "src/expr.h"

#include "built_in_lib/defaultlib.h"

void print_op(operation_t* op) {
    printf("[");

    if (op->a_is_expr_value) {
        printf(" %f ", *((double*)((expr_value_t*)op->value_a)->value));
    } else {
        print_op(op->value_a);
    }

    if (op->value_b != NULL) {
        switch(op->operation) {
            case operation_sum:
                printf("+");
                break;
            case operation_sub:
                printf("-");
                break;
            case operation_mul:
                printf("*");
                break;
            case operation_div:
                printf("/");
                break;
            default:
                printf("Not implemented");
                break;
        }

        if (op->b_is_expr_value) {
            printf(" %f ", *((double*)((expr_value_t*)op->value_b)->value));
        } else {
            print_op(op->value_b);
        }
    }

    printf("]");
}

int main(int argc, char const *argv[])
{

    context_t* main = context_create(NULL);

    List* tok = lexer_lex_line("5+10+5+2");
    int out_int = 0;
    List* ops = expr_compile(tok, main, &out_int);

    for (int j = 0; j < ops->used_length; j++) {
        operation_t* op = list_get(ops, j);
        print_op(op);
    }
    return 0;


    lib_install(main);

    List* parser_state = create_list(sizeof(parser_state_t), 16);

    if (argc == 2) {
        FILE * fp;
        char * line = NULL;
        size_t len = 0;
        size_t read;
        printf("File >> %s\n", argv[1]);
        fp = fopen(argv[1], "r");
        if (fp == NULL)
            exit(EXIT_FAILURE);

        while ((read = getline(&line, &len, fp)) != -1) {
            parser_parse(parser_state, line, main);
        }

        while(parser_state->used_length > 0) {
            parser_parse(parser_state, "\n", main);
        }

        fclose(fp);
        if (line)
            free(line);
    }
    else // If not a file, open the interactive terminal
    while (1) {
        printf(" >> ");

        char line[512];
        gets(line);
        if (strcmp(line, "exit") == 0)
            break;

        parser_parse(parser_state, line, main);
    }

    return 0;
}
