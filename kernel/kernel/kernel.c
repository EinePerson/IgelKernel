#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <stdlib.h>
#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/interrupts.h>
#include <stdio.h>
#include <memory.h>
#include <kernel/memory.h>
#include <kernel/process.h>
#include <kernel.h>

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
};

__attribute__((used, section(".requests")))
struct limine_kernel_address_request address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};
// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

#define HEAP_SIZE 1024 * 20
static char heap[HEAP_SIZE];//20 kb heap for kernel
static struct  process kernel_process;

//entry point
void kmain(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

	if(!memmap_request.response){
        hcf();
    }

    if(framebuffer_request.response->framebuffer_count == 0){
        hcf();
    }
    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    terminal_initialize(framebuffer);

    for(uint32_t i = 0;i < memmap_request.response->entry_count;i++){
        volatile struct limine_memmap_entry* entry = memmap_request.response->entries[i];
        volatile int j = 0;
    }

    setupGDT();
    idt_init();

    struct process* pr_ptr = &kernel_process;
    set_current_process(pr_ptr);
    kernel_process.superviser = true;
    kernel_process.pml4 = get_page_pointer() + PAGE_VIRT_OFFSET;
    kernel_process.pml4_entry_free = 255;
    volatile void* volatile phys = get_phys_addr(heap);

    void* heap_ptr = heap;
    init_heap(heap_ptr,HEAP_SIZE);

    //struct page_existing* page = get_existens_map_default();

    init_page(memmap_request.response);

    printf("Test");

    hcf();
}

struct limine_kernel_address_response* get_addresses() {
    return address_request.response;
}
