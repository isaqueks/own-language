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

void operation_calculate(
    token_type_t operation, 
    variable_type_t a_type, 
    variable_type_t b_type, 

    void* a,
    void* b,
    
    variable_type_t* output_type,
    void* output
);

#endif