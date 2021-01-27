#include <stdio.h>
#include <stdlib.h>

#include "libs/list/list.h"

#include "src/exceptions.h"
#include "src/variable.h"
#include "src/lexer.h"
#include "src/context.h"
#include "src/parser.h"

void testVariables() {

    context_t* window = context_create(NULL);
    context_t* child = context_create(window);

    char* hellow = "Hello World";
    char* test = "This is a test";
    {

        variable_t* string1 = variable_create("string1", String, hellow, strlen(hellow)+1);
        variable_t* ref = variable_create_pointer_from("ref", string1);

        context_add_variable(window, string1);
        context_add_variable(child, ref);

    }
    variable_t* v1 = context_search_variable(window, "string1");
    variable_t* v2 = context_search_variable(window, "ref");

    printf("v1: %s\nv2: %s\n", variable_get_value(v1), variable_get_value(v2));

    variable_free_value(v2);
    variable_assign(v2, test, strlen(test)+1);

    printf("v1: %s\nv2: %s\n", variable_get_value(v1), variable_get_value(v2));

}

int main(int argc, char const *argv[])
{

    // testVariables();
    // return 0;
    context_t* main = context_create(NULL);

    while (1) {
        printf(" >> ");

        char line[1024];
        gets(line);
        if (strcmp(line, "print") == 0)
            break;

        parser_parse(line, main);
    }

    char varname[1024];
    printf(" Variable name: ");
    gets(varname);

    variable_t* x = context_search_variable(main, varname);
    printf("%s = %s\n", varname, variable_get_value(x));

    // List* result = lexer_lex_line(line);
    // if (result == NULL) {
    //     Throw("Unable to continue.");
    // }


    // for(int i = 0; i < result->usedLength; i++) {
    //     token_t* token = list_get(result, i);
    //     printf("TOKEN: !%s!\t\"%s\"\n", token->token, token_type_str[token->type]);
    // }

    return 0;
}
