#include "list.h"
#include <stdio.h>

List *create_list(int objectSize, int length)
{
    List *list = calloc(1, sizeof(List));
    list->objArray = calloc(1, (objectSize) * length);
    list->usedLength = 0;
    list->allocatedLength = length;
    list->objectSize = objectSize;
    return list;
}

void list_addSpace(List *list, unsigned int objectCount)
{
    list->objArray = realloc(list->objArray, (list->allocatedLength + objectCount) * list->objectSize);
    list->allocatedLength += objectCount;
}


void list_add(List *list, void *object_ptr)
{
    if (list->usedLength == list->allocatedLength)
    {
        list_addSpace(list, AUTOADD_COUNT);
    }
    memcpy(get_list_realAddr(list, list->usedLength++), object_ptr, list->objectSize);
}

void *list_get(List *list, int index)
{
    return get_list_realAddr(list, index);
}


void list_remove_at(List* list, int index) {
    memset(get_list_realAddr(list, index), 0x00, list->objectSize);

    for (int i = index+1; i < list->usedLength; i ++) {
        memcpy(get_list_realAddr(list, i-1), get_list_realAddr(list, i), list->objectSize);
    }

    list->usedLength--;
}

void list_set(List* list, int index, void* value) {
    void* ptr = list_get(list, index);
    memcpy(ptr, value, list->objectSize);
}