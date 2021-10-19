#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "expr.h"
#include "primtypes.h"
#include "lexer.h"

typedef void (*type_operation_func)(void* value_a, void* value_b, void* output);

typedef struct {
    token_type_t operation;
    variable_type_t a_type;
    variable_type_t b_type;
    variable_type_t output_type;
    type_operation_func calculate;

} type_operation_t;


#define OPERATIONS 4

type_operation_t type_operation_list[OPERATIONS] = {
    // Sum
    // Number + Number = Number
    { operation_sum, Number, Number, Number, (type_operation_func)Number_sum_Number },
    // Number + String = String
    { operation_sum, Number, String, String, (type_operation_func)Number_sum_String },
    // String + Number = String
    { operation_sum, String, Number, String, (type_operation_func)String_sum_Number },
    // String + String = String
    { operation_sum, String, String, String, (type_operation_func)String_sum_String },
};

void calculate(
    token_type_t operation, 
    variable_type_t a_type, 
    variable_type_t b_type, 

    void* a,
    void* b,
    
    variable_type_t* output_type,
    void* output
);

#endif