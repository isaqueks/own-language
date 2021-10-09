#ifndef _LIST_H_
#define _LIST_H_

#include <stdlib.h>
#include <string.h>

#ifndef AUTOADD_COUNT
    #define AUTOADD_COUNT 16
#endif

typedef struct
{
    void *objArray;
    unsigned int object_size;
    unsigned int used_length;
    unsigned int allocated_length;
} List;

#define get_list_realAddr(list, index) (void *)(&(list->objArray[0]) + ((index) * list->object_size))
#define list_get_value(list, index, T) *(T *)list_get(list, index)

List *create_list(int object_size, int length);
void list_addSpace(List *list, unsigned int objectCount);
void list_add(List *list, void *object_ptr);
void *list_get(List *list, int index);
void list_remove_at(List* list, int index);
void list_set(List* list, int index, void* value);
void list_free(List* list);

#endif