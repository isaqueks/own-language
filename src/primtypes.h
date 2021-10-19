#ifndef PRIMTYPES_H
#define PRIMTYPES_H

#include "lexer.h"
#include "variable.h"

    /* String */
    /* "Math" operations */

void String_sum_String(char* a, char* b, char** output);
void String_subtract_String(char* a, char* b, char** output);
void String_multiply_String(char* a, char* b, char** output);

void String_sum_Number(char* a, double* b, char** output);
void String_subtract_Number(char* a, double* b, char** output);
void String_multiplty_Number(char* a, double* b, char** output);

    /* Methods */

// Converts a string two a Number type (double)
void String_toNumber(char* str, double* output);


    /* Number */
    /* Math operations */


void Number_sum_Number(double* a, double* b, double* output);
void Number_subtract_Number(double* a, double* b, double* output);
void Number_multiply_Number(double* a, double* b, double* output);
void Number_divide_Number(double* a, double* b, double* output);

void Number_sum_String(double* a, char* b, char** output);
void Number_subtract_String(double* a, char* b, char** output);
void Number_multiply_String(double* a, char* b, char** output);

    /* Methods */

// Converts a number to string
// ! Memory will be allocated!
void Number_toString(double* number, char** output);

#endif