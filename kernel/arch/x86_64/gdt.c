#include <kernel/gdt.h>

#define GDT_ENTRY_SIZE 8
#define GDT_ENTRY_COUNT 7//TSS takes up two slots

void setGdt(uint16_t limit,uint64_t base);

void reloadSegments();
void setTSS();

struct TaskStateSegment{
    unsigned int	res;

    unsigned long	rsp0;
    unsigned long	rsp1;
    unsigned long	rsp2;

    unsigned long	res2;

    unsigned long   IST1;
    unsigned long   IST2;
    unsigned long   IST3;
    unsigned long   IST4;
    unsigned long   IST5;
    unsigned long   IST6;
    unsigned long   IST7;

    unsigned long   res3;
    unsigned short  res4;
    unsigned short  IOPB;
}__attribute__((packed));

struct GDT{
    uint64_t base;
    uint32_t limit;
    uint8_t access_byte;
    uint8_t flags;
} __attribute__((packed));;

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void encodeGdtEntry(uint8_t *target, struct GDT source)
{
    // Check the limit to make sure that it can be encoded
    if (source.limit > 0xFFFFF) {
        //kerror("GDT cannot encode limits larger than 0xFFFFF");
        //TODO err
    }

    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] = (source.limit >> 16) & 0x0F;

    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;
	if(source.access_byte & 0x89){
		target[8] = (source.base >> 32) & 0xFF;
		target[9] = (source.base >> 40) & 0xFF;
		target[10] = (source.base >> 48) & 0xFF;
		target[11] = (source.base >> 56) & 0xFF;
	}

    // Encode the access byte
    target[5] = source.access_byte;

    // Encode the flags
    target[6] |= (source.flags << 4);

}

static volatile char gdtPtr[GDT_ENTRY_SIZE * GDT_ENTRY_COUNT];
static volatile struct TaskStateSegment tss;

void setupGDT(){
	struct TaskStateSegment* tssPtr = &tss;
    /*if((ptr >> 32) != 0){
        //TODO err ptr must be in fist 4 Gib of mem
    }*/

    struct GDT null_des = {0,0,0,0};
    encodeGdtEntry(gdtPtr,null_des);

    struct GDT KC_des = {0,0xFFFFF,0x9A,0xA};
    encodeGdtEntry(gdtPtr + GDT_ENTRY_SIZE,KC_des);

    struct GDT KD_des = {0,0xFFFFF,0x92,0xC};
    encodeGdtEntry(gdtPtr + GDT_ENTRY_SIZE * 2,KD_des);

    struct GDT UC_des = {0,0xFFFFF,0xFA,0xA};
    encodeGdtEntry(gdtPtr + GDT_ENTRY_SIZE * 3,UC_des);

    struct GDT UD_des = {0,0xFFFFF,0xF2,0xC};
    encodeGdtEntry(gdtPtr + GDT_ENTRY_SIZE * 4,UD_des);

    //TODO add Task State Segment

    struct GDT TSS_des = {(uint64_t)tssPtr,sizeof(tss) - 1,0x89,0x0};
	tssPtr->rsp0 = 0xffff800007f660a0;
	tssPtr->rsp1 = 0xffff800007f660a0;
	tssPtr->rsp2 = 0xffff800007f660a0;
    encodeGdtEntry(gdtPtr + GDT_ENTRY_SIZE * 5,TSS_des);

    setGdt(GDT_ENTRY_SIZE * GDT_ENTRY_COUNT,(uint64_t) gdtPtr);

	setTSS();

	reloadSegments();

	/*asm (
        "ltr %0" :: "r"((uint16_t)0x28)
    );*/

	//__asm__ volatile (".intel_syntax noprefix;mov ax,32;ltr ax;.att_syntax;" : : /*"i"(GDT_ENTRY_SIZE * 4)*/:"ax"); // load the TSS
}