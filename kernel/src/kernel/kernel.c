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
#include <kernel/RSDP.h>
#include <kernel.h>
#include <kernel/APIC.h>
#include <kernel/MSR.h>
#include <util.h>
#include <drivers/keyboard.h>
#include <drivers/PCI.h>
#include <drivers/USB.h>

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

__attribute__((used, section(".requests")))
struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 2,
    .flags = LIMINE_SMP_X2APIC,//Enable X2APIC
};

__attribute__((used, section(".requests")))
struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

uint64_t get_kernel_virt_offset() {
    return address_request.response->virtual_base - address_request.response->physical_base;
}

// Halt and catch fire function.
void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

void* map_page_APIC(struct Parsed_MADT madt) {
    struct page_existing* pep = get_existens_map(get_page_pointer() + PAGE_VIRT_OFFSET,1);
    int16_t pml4 = 0;
    for(int i = 511;i > 0;i--) {
        if(pep->exists_ptr[i] == 0) {
            pml4 = i;
            break;
        }
    }

    uint64_t offset = address_request.response->virtual_base - address_request.response->physical_base;
    uint8_t flags = 0b11011;
    uint64_t virt = 0xFFFF;
    virt <<= 9;
    virt += pml4;
    virt <<= 9;

    volatile void* pdpt = malloc_same(4096);
    void* cr3 = get_page_pointer();
    cr3 += PAGE_VIRT_OFFSET;
    ((uint64_t**) cr3)[pml4] = (uint64_t*) ((uint64_t) ((uint8_t*) pdpt - offset) | flags);

     volatile void* pd = malloc_same(4096);
    ((uint64_t**) pdpt)[511] = (uint64_t*) ((uint64_t) ((uint8_t*) pd - offset) | flags);
    virt += 511;
    virt <<= 9;

    void* pt = malloc_same(4096);
    ((uint64_t**) pd)[511] = (uint64_t*) ((uint64_t) ((uint8_t*) pt - offset) | flags);
    virt += 511;
    virt <<= 9;

    /*if((madt.lapic_addr & 0xFFFFFFFFFFFF000) != madt.lapic_addr) {
        printf("LAPIC address unaligned");
        hcf();
    }*/

    if((madt.IOAPIC_addr & 0xFFFFFFFFFFFF000) != madt.IOAPIC_addr) {
        printf("IOAPIC address unaligned");
        hcf();
    }

    //((uint64_t**) pt)[1] = (uint64_t*) (madt.lapic_addr | flags);

    ((uint64_t**) pt)[0] = (uint64_t*) (madt.IOAPIC_addr | flags);

    virt <<= 12;

    return (void*) virt;
}

void pic_disable(void) {
    outb(0x21, 0xFF);
    outb(0xA1, 0xff);
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

	if(memmap_request.response == 0){
        hcf();
    }

    if(framebuffer_request.response->framebuffer_count == 0){
        hcf();
    }

    if(smp_request.response == 0 || (smp_request.response != 0 && (smp_request.response->flags & LIMINE_SMP_X2APIC) != 1)) {
        hcf();
    }

    if(rsdp_request.response == 0) {
        hcf();
    }

    pic_disable();

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    terminal_initialize(framebuffer);

    setupGDT();
    idt_init();

    struct process* pr_ptr = &kernel_process;
    set_current_process(pr_ptr);
    kernel_process.superviser = true;
    kernel_process.pml4 = get_page_pointer() + PAGE_VIRT_OFFSET;
    kernel_process.pml4_entry_free = 255;

    void* heap_ptr = heap;
    init_heap(heap_ptr,HEAP_SIZE);

    init_page(memmap_request.response);

    setXSDP(rsdp_request.response->address);

    //MASK PIC
    //outb(0x20, 0x20); // End of interrupt for master PIC
    //outb(0xA0, 0x20); // End of interrupt for slave PIC
    /*outb(0x21, 0xFF);  // Mask all interrupts on master PIC
    outb(0xA1, 0xFF);  // Mask all interrupts on slave PIC
    outb(0x20, 0x11);  // Start PIC initialization
    outb(0xA0, 0x11);  // Start slave PIC initialization
    outb(0x21, 0x20);  // Map master PIC
    outb(0xA1, 0x28);  // Map slave PIC
    outb(0x21, 0x04);  // Tell master PIC there's a slave at IRQ2
    outb(0xA1, 0x02);  // Tell slave PIC its cascade identity
    outb(0x21, 0x01);  // Put PICs in 8086 mode
    outb(0xA1, 0x01);  // Put slave PIC in 8086 mode*/

    struct FADT* fadt = find_FADT();
    struct Parsed_MADT parsed = parsed_madt();
    PCIe_init();
    usb_init();
    void* apic_table = map_page_APIC(parsed);
    write_msr(0x80F,read_msr(0x80F) | 0x1FF);//Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts

    uint32_t msr_val = read_msr(0x01B);
    msr_val |= (1 << 11);
    write_msr(0x1B,msr_val);

    IOAPIC_setup(apic_table,smp_request.response->bsp_lapic_id);

    init_keyboard();

    printf("Test\n");

    while (true){}
    hcf();
}

struct limine_kernel_address_response* get_addresses() {
    return address_request.response;
}
