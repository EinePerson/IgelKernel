//
// Created by igel on 15.01.25.
//

#ifndef LISTS_H
#define LISTS_H

#include <stdint.h>

/*struct LinkedListNode{
	struct LinkedListNode* next;
    void* content;
};

struct LinkedListHead{
	struct LinkedListNode* first;
};*/

struct ArrayList{
	void* content;
    uint64_t size;
	uint64_t capacity;
    uint64_t bucketSize;
};

#endif //LISTS_H
