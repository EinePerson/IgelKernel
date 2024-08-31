#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <limine.h>

struct process;

struct free_memory{
    struct free_memory* next_block;
    uint64_t size;
};

struct used_metadata{
    uint64_t size;
};

struct page_table{
    void* page[512];
} __attribute__((packed,aligned(4096))) ;

struct page_directory{
    struct page_table* tables[512];
} __attribute__((packed,aligned(4096)));

struct page_dir_pointer{
    struct page_directory* dirs[512];
} __attribute__((packed,aligned(4096)));

//this is where cr3 points to
struct page_level_4{
    struct page_dir_pointer* dirs[512];
} __attribute__((packed,aligned(4096)));

struct page_existing {
    struct page_existing* exists_ptr[512];
};

struct memory{
    void* ptr;
    uint64_t size;
    uint8_t* used_bits;//Every page(4kb) has a bit that marks its status
    uint32_t used_bits_size;
}__attribute__((packed));

void* malloc(uint64_t size);

void free(void* ptr);

void init_heap(void* start,uint64_t size);

//This is a heap for objects of a predefined size
void init_same(void* mem,uint64_t mem_size,uint64_t obj_size);

void* malloc_same();
void free_same(void* ptr);
void find_next_free_same();


//The size is in pages(4096 bytes)
//void** alloc_page(uint64_t size);

void init_page(struct limine_memmap_response* memmap);

void* alloc_next_page(uint8_t flags);
void* map_next_to(void* mem,uint8_t flags);
void find_next_free();
void find_free_virt();
//this functions checks weather the specific address is unused,if so it will alocate the unalocated parts of it, returns if this is free
bool check_free_and_alloc();

void set_current_process(struct process* process);

struct page_dir_pointer* get_pml4();
struct page_directory* get_pdpt();
struct page_table* get_pd();
void* get_pt();