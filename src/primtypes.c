#include "primtypes.h"
#include <stdio.h>
#include <string.h>

/* String */
/* "Math" operations */

char* String_sum(char* a, char* b) {
    char* out = malloc(strlen(a)+strlen(b)+1);
    strcpy(out, a);
    strcat(out, b);
    return out;
}

/* Methods */

double String_toNumber(char* str) {
    return strtod(str, NULL);
}


/* Number */
/* Math operations */

double Number_sum(double a, double b) {
    return a+b;
}

double Number_subtract(double a, double b) {
    return a-b;
}

double Number_multiply(double a, double b) {
    return a*b;
}

double Number_divide(double a, double b) {
    return a/b;
}


/* Methods */

char* Number_toString(double number) {
    char buffer[48];
    sprintf(buffer, "%s", number);
    int len = strlen(buffer);
    char* str = malloc(len+1);
    strcpy(str, buffer);
    return str;
}