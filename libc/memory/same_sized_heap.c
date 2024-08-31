//
// Created by igel on 29.08.24.
//

#include <stdbool.h>
#include <stdint.h>

#include <memory.h>
#include <string.h>

static void* mem;
static uint8_t* mem_used;
static uint64_t mem_used_size;
static uint64_t next_free;
static uint64_t obj_size;

void init_same(void *new_mem, uint64_t mem_size, uint64_t new_obj_size) {
    mem = new_mem;
    obj_size = new_obj_size;
    mem_used_size = mem_size / obj_size;
    mem_used = malloc(mem_used_size);
    next_free = 0;
    memset(mem_used,0,mem_used_size);
}

void* malloc_same() {
    void* ret = mem + next_free * obj_size;
    mem_used[next_free / 8] |= 1 << next_free % 8;
    find_next_free_same();
    return ret;
}

void free_same(void *ptr) {
    ptr -= (uint64_t)mem;
    uint64_t index = (uint64_t) ptr / obj_size;
    uint64_t arr_index = index / 8;
    uint8_t offset = index % 8;

    mem_used[arr_index] &= !(1 << offset);
}

void find_next_free_same() {
    next_free++;
    uint64_t arr_index = next_free / 8;
    uint8_t offset = next_free % 8;

    while (arr_index < mem_used_size) {
        while (offset < 8) {
            if(((mem_used[arr_index] >> offset) & 1) == 0) {
                next_free = arr_index * 8 + offset;
                return;
            }
            offset++;
        }
        arr_index++;
    }
}
