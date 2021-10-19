#include "primtypes.h"
#include "errors.h"
#include <stdio.h>
#include <string.h>

/* String */
/* "Math" operations */

void String_sum_String(char* a, char* b, char** output) {
    char* out = malloc(strlen(a)+strlen(b)+1);
    strcpy(out, a);
    strcat(out, b);
    *output = out;
}

void String_subtract_String(char* a, char* b, char** output) {
    RUNTIME_ERR(TODO_ERROR, "Not implemented");
}

void String_multiply_String(char* a, char* b, char** output) {
    int a_len = strlen(a);
    int b_len = strlen(b);

    // He * AA
    // HAAeAA

    char* out = malloc(a_len*b_len+1);
    int idx = 0;

    for (int i = 0; i < a_len; i++) {
        out[idx++] = a[i];

        for (int j = 0; j < b_len; j++) {
            out[idx++] = b[j];
        }
    }

    out[idx] = 0x00;
    *output = out;
}

void String_sum_Number(char* a, double* b, char** output) {
    char* b_str;
    Number_toString(b, &b_str);
    char* out = malloc(strlen(a)+strlen(b_str)+1);
    strcpy(out, a);
    strcat(out, b_str);

    free(b_str);
    *output = out;
}

void String_subtract_Number(char* a, double* b, char** output) {
    RUNTIME_ERR(TODO_ERROR, "Not implemented");
}

void String_multiplty_Number(char* a, double* b, char** output) {
    RUNTIME_ERR(TODO_ERROR, "Not implemented");
}

/* Methods */

void String_toNumber(char* str, double* output) {
    *output = strtod(str, NULL);
}


/* Number */
/* Math operations */

void Number_sum_Number(double* a, double* b, double* output) {
    *output = (*a + *b);
}

void Number_subtract_Number(double* a, double* b, double* output) {
    *output = (*a - *b);
}

void Number_multiply_Number(double* a, double* b, double* output) {
    *output = (*a * (*b));
}

void Number_divide_Number(double* a, double* b, double* output) {
    *output = (*a / *b);
}

void Number_sum_String(double* a, char* b, char** output) {
    char* a_str;
    Number_toString(a, &a_str);
    char* out = malloc(strlen(a_str)+strlen(b)+1);
    strcpy(out, a_str);
    strcat(out, b);
    *output = out;
}

void Number_subtract_String(double* a, char* b, char** output) {
    RUNTIME_ERR(TODO_ERROR, "Not implemented");
}

void Number_multiply_String(double* a, char* b, char** output) {
    RUNTIME_ERR(TODO_ERROR, "Not implemented");
}

/* Methods */

void Number_toString(double* number, char** output) {
    char buffer[48];
    sprintf(buffer, "%s", number);
    int len = strlen(buffer);
    char* str = malloc(len+1);
    strcpy(str, buffer);
    *output = str;
}