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


int main(int argc, char const *argv[])
{

    context_t* main = context_create(NULL);

    List* tok = lexer_lex_line("1*2*3/5");
    int out_int = 0;
    List* ops = expr_parse_linear(tok, main, &out_int);
    expr_print_linear(ops);
    printf("mount tree\n");
    List* tree = expr_mount_tree(ops, 1);
    printf("tree done:\n\n");
    expr_print_tree(tree);
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
        if (fp == NULL) {
            exit(EXIT_FAILURE);
        }

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
