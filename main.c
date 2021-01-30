#include <stdio.h>
#include <stdlib.h>

#include "libs/list/list.h"

#include "src/exceptions.h"
#include "src/variable.h"
#include "src/lexer.h"
#include "src/context.h"
#include "src/parser.h"
#include "src/function.h"

void print_internal(void* ctxptr) {
    context_t* ctx = (context_t*)ctxptr;
    variable_t* arg_str = context_search_variable(ctx, "str");

    printf("%s\n", (char*)variable_get_value(arg_str));
}

int main(int argc, char const *argv[])
{

    context_t* main = context_create(NULL);

    {
        variable_t* arg_str = variable_create("str", String, NULL, 0);
        native_function_handle_t printhandle = print_internal;
        List* args = create_list(sizeof(variable_t*), 1);
        list_add(args, &arg_str);
        function_t* print_func = function_create_native("print", args, printhandle);
        context_add_function(main, print_func);
    }

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
