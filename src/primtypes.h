#ifndef PRIMTYPES_H
#define PRIMTYPES_H

#include "lexer.h"
#include "variable.h"

    /* String */
    /* "Math" operations */

// Concatenates two strings
// ! Uses malloc()
char* String_sum(char* a, char* b);
    /* Methods */

// Converts a string two a Number type (double)
double String_toNumber(char* str);


    /* Number */
    /* Math operations */


double Number_sum(double a, double b);
double Number_subtract(double a, double b);
double Number_multiply(double a, double b);
double Number_divide(double a, double b);

    /* Methods */

// Converts a number to string
// ! Memory will be allocated!
char* Number_toString(double number);

#endif