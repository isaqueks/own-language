#include "variable.h"
#include "exceptions.h"
#include <string.h>
#include <stdbool.h>

variable_type_t variable_get_type_by_string(char* str) {

    if (strcmp(str, "String") == 0)
        return String;

    else if (strcmp(str, "Number") == 0)
        return Number;

    return Any;
}

variable_t* variable_create(char* name, variable_type_t var_type,
void* value, uint32_t value_length) {

    variable_t* var = malloc(sizeof(variable_t));
    if (var == NULL)
        Throw("Not enough memory!");

    var->is_pointer = false;
    var->name = name;
    var->type = var_type;
    var->value_pointer = value;
    var->value_length = value_length;

    return var;

}

variable_t* variable_create_pointer_from(char* name, variable_t* other) {

    if (other == NULL || other->value_pointer == NULL)
        Throw("*other or it's value is NULL!");

    variable_t* var = variable_create(name, other->type,
    other, other->value_length);
    var->is_pointer = true;
    return var;
}

variable_t* variable_create_from(char* name, variable_t* other) {

    if (other == NULL || other->value_pointer == NULL)
        Throw("*other or it's value is NULL!");

    variable_t* var = variable_create(name, other->type,
    NULL, other->value_length);

    if (other->type != Object && other->type != Array) {
        var->value_pointer = calloc(1, other->value_length);
        memcpy(var->value_pointer, variable_get_value(other), var->value_length);
    }
    else {
        var->value_pointer = other;
        var->is_pointer = true;
    }

    return var;
}

void variable_free_value(variable_t* var) {

    var->value_length = 0;
    if (var->value_pointer != NULL)
    free(var->value_pointer);

    // * No need to clean the real value.
    // * The real value will be cleaned
    // * when variable_free_value would
    // * be called for origin variable.
    // if (!var->is_pointer) {
    //     free(var->value_pointer);
    //     var->value_pointer = NULL;
    // }
    // else {
    //     // Clean the REAL value address
    //     variable_free_value((variable_t*)var->value_pointer);
    // }
    // If not, there's no allocated memory for the pointer variable.
    // The value_pointer itself is already 4 bytes size;
}

void variable_assign(variable_t* var, void* value, uint32_t value_length) {

    if (!var->is_pointer && var->value_pointer != NULL)
        // For preventing memory leaks
        // But, if it's a pointer, there's no allocated memory
        Throw("Value is not NULL! Must be freed before variable_assign!");

    var->value_length = value_length;

    // ! If it's a pointer, 
    // if (var->is_pointer)
    //     variable_assign((variable_t*)var->value_pointer, value, value_length);
    // else
    //     var->value_pointer = value;

    // Break pointer and assign a value.
    // For assigning as pointer, use variable_assign_as_pointer
    var->value_pointer = value;
    var->is_pointer = false;
}

void variable_assign_as_pointer(variable_t* var, variable_t* other) {
    if (var->value_pointer != NULL)
        Throw("Value is not NULL! Must be freed before variable_assign_as_pointer!");
    
    var->is_pointer = true;
    var->value_length = other->value_length;
    var->value_pointer = other;
}

void* variable_get_value(variable_t* var) {

    if (var == NULL || var->value_pointer == NULL)
        Throw("*var or it's value is NULL!");

    if (var->is_pointer) // Get the value itself, not the address of the referenced variable.
        return variable_get_value((variable_t*)var->value_pointer);

    return var->value_pointer;
}

void variable_free_props(variable_t* var) {
    free(var->name);
    variable_free_value(var);
}

void variable_free(variable_t* var) {
    variable_free_props(var);
    free(var);
}