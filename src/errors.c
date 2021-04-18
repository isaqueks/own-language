#include "errors.h"
#include <stdio.h>

void __RUNTIME_ERR(char* x, char* y, char* filename, int line) {
    printf("[%s:%d] %s: %s.", filename, line, x, y);
    while(1);
}

void __ERR(char* x, char* y, char* token_type, char* token, int i, char* filename, int line)  {
    printf("[%s:%d] %s: %s: %s (\"%s\"[%d]) received.", filename, line, x, y, token_type, token, i);
    while(1);
}