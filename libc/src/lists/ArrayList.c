//
// Created by igel on 09.03.25.
//

#include <string.h>

#include "lists.h"
#include "memory.h"

#define ARRAY_LIST_STANDARD_BUCKET_SIZE 10

void ArrayList_init(struct ArrayList* list) {
    if (list->bucketSize == 0)list->bucketSize = ARRAY_LIST_STANDARD_BUCKET_SIZE;

    list->size = 0;
    list->content = malloc(sizeof(void*) * list->bucketSize);
}

void ArrayList_destroy(struct ArrayList* list) {
    free(list->content);
}

void ArrayList_clear(struct ArrayList* list) {
    memset(list->content,0,sizeof(void*) * list->size);
}

void extend(struct ArrayList * list) {
    void* newField = malloc((list->capacity + list->bucketSize) * sizeof(void*));
    memcpy(newField,list->content,sizeof(void*) * list->size);
    free(list->content);
    list->content = newField;
    list->capacity = list->capacity + list->bucketSize;
}

void ArrayList_add(struct ArrayList* list, void* item) {
    if (list->capacity >= list->size + 1)extend(list);
    ((void**) list->content)[list->size] = item;
    list->size++;
}
