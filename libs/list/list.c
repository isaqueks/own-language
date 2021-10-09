#include "list.h"
#include <stdio.h>

List *create_list(int object_size, int length)
{
    List *list = calloc(1, sizeof(List));
    list->objArray = calloc(1, (object_size) * length);
    list->used_length = 0;
    list->allocated_length = length;
    list->object_size = object_size;
    return list;
}

void list_addSpace(List *list, unsigned int objectCount)
{
    list->objArray = realloc(list->objArray, (list->allocated_length + objectCount) * list->object_size);
    list->allocated_length += objectCount;
}


void list_add(List *list, void *object_ptr)
{
    if (list->used_length == list->allocated_length)
    {
        list_addSpace(list, AUTOADD_COUNT);
    }
    memcpy(get_list_realAddr(list, list->used_length++), object_ptr, list->object_size);
}

void *list_get(List *list, int index)
{
    return get_list_realAddr(list, index);
}


void list_remove_at(List* list, int index) {
    memset(get_list_realAddr(list, index), 0x00, list->object_size);

    for (int i = index+1; i < list->used_length; i ++) {
        memcpy(get_list_realAddr(list, i-1), get_list_realAddr(list, i), list->object_size);
    }

    list->used_length--;
}

void list_set(List* list, int index, void* value) {
    void* ptr = list_get(list, index);
    memcpy(ptr, value, list->object_size);
}

void list_free(List* list) {
    free(list->objArray);
    free(list);
}