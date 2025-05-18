#include <stdint.h>
#include <string.h>

void* memset(void* bufptr, int value, size_t size) {
	unsigned char* buf = (unsigned char*) bufptr;
	for (size_t i = 0; i < size; i++)
		buf[i] = (unsigned char) value;
	return bufptr;
}

void* memsetLong(void* bufptr,uint64_t value, size_t size) {
	uint64_t* buf = (uint64_t*) bufptr;
	size_t end = size / 8;
	for (size_t i = 0; i < end; i++)
		buf[i] = (uint64_t) value;
	return bufptr;
}
