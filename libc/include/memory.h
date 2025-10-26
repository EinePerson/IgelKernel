#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <limine.h>

///Paging Flags

#define PAGING_PRESENT_FLAG 1
#define PAGING_WRITE_FLAG 1 << 1
#define PAGING_ALL_ACCESS_FLAG 1 << 2
#define PAGING_WRITE_THROUGH_FLAG 1 << 3
#define PAGING_CACHE_DISABLED_FLAG 1 << 4
#define PAGING_ACCESSED_FLAG 1 << 5
#define PAGING_DIRTY_FLAG 1 << 6
///only for pdpt and pd
#define PAGING_PAGE_SIZE_FLAG 1 << 7
///only for pt
#define PAGING_PAT_FLAG 1 << 7
#define PAGING_GLOBAL_FLAG 1 << 8
#define PAGING_FLAG_NOT_EXECUTABLE 1 << 63

#define DEFAULT_KERNEL_PAGE_FLAGS PAGING_WRITE_FLAG | PAGING_PRESENT_FLAG
#define DEFAULT_USER_PAGE_FLAGS PAGING_ALL_ACCESS_FLAG | PAGING_WRITE_FLAG | PAGING_PRESENT_FLAG
///this defines the bits that are not part of the address itself
#define SECURITY_FLAG_MASK 0x8000000000000FFF

struct process;
struct Memory_Virtual_Address;

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

/*
struct space_memory_translator_pair {
    uint64_t virt_addr;
    uint64_t phys_addr;
    struct space_memory_translator_pair* next;///this uses a linked list approach as the list should be mostly unused so a singular value is simpler
};

struct space_memory_translator_bucket {
    uint8_t length;
    struct space_memory_translator_pair* first;
};*/

/**
 * @Deprecated
 * This is a memory translator which maps virtual addresses and physical ones meaning that each pair is present twice so that one can act on the values only requiring one of physical and virtual address<br>
 * create one by calling create_address_translator()<br>
 * the following functions may be called to alter the value,each function is present twice to allow access for each possible type of address
 * void addEntry
 *
 * bool containsPhysicalAddress
 * bool containsVirtualAddress
 *
 * void* getByPhysicalAddress
 * void* getByVirtualAddress
 *
 * void removeEntryByVirtualAddress
 * void removeEntryByPhysicalAddress
 */
struct space_memory_translator;

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

void init_page(struct limine_memmap_response* memmap, const struct limine_kernel_address_response* address_range);

void* alloc_next_page(uint8_t flags);
void* map_next_to(void* mem,uint64_t flags);
void find_next_free();
void find_free_virt();
//this functions checks weather the specific address is unused,if so it will alocate the unalocated parts of it, returns if this is free
bool check_free_and_alloc(struct Memory_Virtual_Address* addr);
bool check_free_and_alloc_structures(struct Memory_Virtual_Address* addr,uint16_t flags);
bool check_free_and_alloc_next();

void set_current_process(struct process* process);

/*struct space_memory_translator* create_address_translator();
void addEntry(struct space_memory_translator* translator,void* virtual_address,void* physical_address);

bool containsPhysicalAddress(struct space_memory_translator* translator,void* phys);
bool containsVirtualAddress(struct space_memory_translator* translator,void* virt);

void* getByPhysicalAddress(struct space_memory_translator* translator,void* physical_address);
void* getByVirtualAddress(struct space_memory_translator* translator,void* virtual_address);

void removeEntryByVirtualAddress(struct space_memory_translator* translator,void* virt);
void removeEntryByPhysicalAddress(struct space_memory_translator* translator,void* phys);*/

struct page_dir_pointer* get_pml4(uint16_t pml4_id);
struct page_directory* get_pdpt(uint16_t pml4_id,uint16_t pdpt_id);
struct page_table* get_pd(uint16_t pml4_id,uint16_t pdpt_id,uint16_t pd_id);
void* get_pt(uint16_t pml4_id,uint16_t pdpt_id,uint16_t pd_id,uint16_t pt_id);
struct Memory_Virtual_Address get_current_process_address();

void* alloc_physical_page_range(uint64_t length);

void* alloc_next_free_page_directory();

//void init_translator();

//void* get_physical_address(void* virt,uint64_t cr3);

struct address_space_translator_entry {
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t length;
    //uint64_t all_flags;
    /*uint16_t flags_pml4;
    uint16_t flags_pdpt;
    uint16_t flags_pd;
    uint16_t flags_pt;*/
    struct address_space_translator_entry* next;
};

struct address_space_translator{
    uint32_t size;
    struct address_space_translator_entry* first;
    struct address_space_translator_entry* cache;
};

void translator_init_kernel_space(struct address_space_translator* trans,uint64_t stack_offset,uint64_t instruction_offset);

void translator_remove_entry_physical(struct address_space_translator* trans,void* phys,uint64_t length);
void translator_remove_entry_virtual(struct address_space_translator* trans,void* virt,uint64_t length);

bool translator_contains_physically(struct address_space_translator* trans,void* phys);
bool translator_contains_virtual(struct address_space_translator* trans,void* virt);

//uint16_t translator_get_flags_physically(struct address_space_translator* trans,void* phys,uint8_t index);
//uint16_t translator_get_flags_virtually(struct address_space_translator *trans, void *virt,uint8_t index);

uint16_t translator_get_level_flags(struct address_space_translator* translator,void* virt,uint8_t level);
uint64_t translator_get_flags(struct address_space_translator* translator,void* virt);

void* translator_translate_physical(struct address_space_translator* trans,void* phys);
void* translator_translate_virtual(struct address_space_translator* trans,void* virt);

/**
 * @param trans the translator
 * @param addrStr the addres to search for
 * @param level how deep the check has to be performed level 1 is pml4,2 is pdpt so,3 is pd,4 is pt
 * @return wether the address is present, this requirement has to be met up to the level meaning that the specific address may be free but a certain range before may be taken
*/
bool translator_contains_virtual_addr(struct address_space_translator* trans,struct Memory_Virtual_Address* addrStr,uint8_t level);
/**
 * same as translator_contains_virtual_addr() but with address as uint64_t
*/
bool translator_contains_immideate_virtual_addr(struct address_space_translator* trans,uint64_t virt,uint8_t level);
void translator_add_entry(struct address_space_translator* trans,void* virt,void* phys,uint64_t length);

uint64_t translateAddrToInt(struct Memory_Virtual_Address* addrStr);

struct address_space_translator* create_translator();