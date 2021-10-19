
#include "operations.h"
#include "errors.h"

void calculate(
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