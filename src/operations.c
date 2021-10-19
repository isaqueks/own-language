
#include "operations.h"
#include "errors.h"

#define OPERATIONS (sizeof(type_operation_list)/sizeof(type_operation_list[0]))

type_operation_t type_operation_list[] = {
    // Sum
    // Number + Number = Number
    { operation_sum, Number, Number, Number, (type_operation_func)Number_sum_Number },
    // Number + String = String
    { operation_sum, Number, String, String, (type_operation_func)Number_sum_String },
    // String + Number = String
    { operation_sum, String, Number, String, (type_operation_func)String_sum_Number },
    // String + String = String
    { operation_sum, String, String, String, (type_operation_func)String_sum_String },

    // Multiplication
    // Number * Number = Number
    { operation_mul, Number, Number, Number, (type_operation_func)Number_multiply_Number },
};

void operation_calculate(
    token_type_t operation, 
    variable_type_t a_type, 
    variable_type_t b_type, 

    void* a,
    void* b,
    
    variable_type_t* output_type,
    void* output
) {

    for (int i = 0; i < OPERATIONS; i++) {
        type_operation_t* op = &type_operation_list[i];
        if (
            op->operation == operation &&
            op->a_type == a_type && 
            op->b_type == b_type
        ) {
            op->calculate(a, b, output);
            return;
        }
    }

    Throw("No operation found!");

}