#ifndef VARIABLE_H
#define VARIABLE_H

#include <inttypes.h>
#include <stdbool.h>

typedef enum {

    Number,
    String,
    Array,
    Object,
    Any

} variable_type_t;

variable_type_t variable_get_type_by_string(char* str);

typedef struct {
    char* name;                 // length = strlen(name)
    bool is_pointer;
    variable_type_t type;
    void* value_pointer;        // can be a binary value
    uint32_t value_length;      
} variable_t;


/*
    Default variable constructor.
    @param name: The name for the new variable
    @param var_type: The type for the new variable.
    @param value: The value's pointer for the new variable (must already be allocated).
    @param value_length: The size (in bytes) of the value.
*/
variable_t* variable_create(char* name, variable_type_t var_type,
void* value, uint32_t value_length);

/*
    Creates a pointer variable from other variable.
    @param name: The name for the new variable
    @param other: The other variable.
*/
variable_t* variable_create_pointer_from(char* name, variable_t* other);

/*
    Creates a variable from other variable's value. 
    If it's an Object or Array, then a pointer will be created.
    @param name: The name for the new variable
    @param other: The other variable
*/
variable_t* variable_create_from(char* name, variable_t* other);

/*
    Free's allocated memory for a variable's value 
    and set it's value pointer to NULL.
    @param var: The variable_t*
*/
void variable_free_value(variable_t* var);

/*
    Assign a value to an existing variable.   
    !! Memory will be allocated if value is not NULL.
    @param var: The variable_t*
    @param value: (Must already be allocated) will be the new pointer for the var's value
    @param value_length: The size of the new value (in bytes);
*/
void variable_assign(variable_t* var, void* value, uint32_t value_length);

/*
    Create a pointer to an existing variable.   
    !! Memory will be allocated if value is not NULL.
    @param var: The variable_t*
    @param other: The other variable
*/
void variable_assign_as_pointer(variable_t* var, variable_t* other);

/*
    Returns the value of the variable (pointer).
    @param var: The variable_t*
*/
void* variable_get_value(variable_t* var);

#endif