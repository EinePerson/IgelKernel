//
// Created by igel on 05.09.24.
//

#include <kernel/APIC.h>
#include <kernel/MSR.h>

#include "memory.h"
#include "stdio.h"
#include "string.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"

extern void init_PIT();
extern void PIT_start();
extern uint64_t get_time();

#define LVT_R_MASK_INTERRUPTS 1 << 16

static uint64_t ticks_per_ms = -1;

//only x2APIC is used
enum LAPIC_Registers{
    ID = 0x2,
    VERSION = 0x3,
    TASK_PRIORITY= 0x8,
    ARBITRATION_PRIORITY = 0x9,
    PROCESS_PRIORITY = 0xA,
    EOI = 0xB,
    REMOTE_READ = 0xC,
    LOGICAL_DESTINATION = 0xD,
    //DESTINATION_FORMAT = 0xE0,
    SPURIOUS_INTERRUPT_VR = 0xF,

    ERROR_STATUS_REGISTER = 0x28,

    INTERRUPT_COMMAND_REGISTER = 0x30,

    LVT_TIMER = 0x32,
    LVT_THERMAL = 0x33,
    LVT_PERFORMANCE_COUNTER = 0x34,
    LINT0_R = 0x35,
    LINT1_R = 0x36,
    LVT_E_R = 0x37,
    INIT_COUNT_REGISTER = 0x38,
    CURRENT_COUNT_TIMER = 0x39,
    DIVIDE_CONFIGURATION_REGISTER = 0x3E,
    SELF_IPI = 0x3F
};

enum Timer_Mode{
    One_Shot = 0b00,
    Periodic = 0b01,
    TSC_deadline = 0b10,
};

struct Timer_waits{
    uint64_t tID;
    uint64_t relative_time;
    struct Timer_waits *next;
};

void set_APIC_Timer_Interrupt(enum Timer_Mode mode,uint8_t interrupt_id){
    uint32_t msr_val = read_from_LAPIC(LVT_TIMER);
    msr_val &= 0xFFFCFF00;
    msr_val |= interrupt_id;
    msr_val |= (mode & 0x3) << 16;
    write_to_LAPIC(LVT_TIMER,msr_val);
}

uint32_t cpuReadIoApic(void *ioapicaddr, uint32_t reg){
    uint32_t volatile *ioapic = (uint32_t volatile *)ioapicaddr;
    ioapic[0] = (reg & 0xff);
    return ioapic[4];
}

void cpuWriteIoApic(void *ioapicaddr, uint32_t reg, uint32_t value){
    uint32_t volatile *ioapic = (uint32_t volatile *)ioapicaddr;
    ioapic[0] = (reg & 0xff);
    ioapic[4] = value;
}

void IOAPIC_setup(void *address,uint8_t APIC_ID) {
    for(uint8_t i = 0 ;i < 16;i++) {
        uint32_t val = (0x30 + i) & 0x00FF;
        val |= (0x1 << 16) * (i == 2);//for some reason the cascade is triggered and is thus masked here
        cpuWriteIoApic(address,0x10 + 2 * i,val);
        cpuWriteIoApic(address,0x14 + 2 * i,APIC_ID << 24);
    }
}

void write_to_LAPIC(enum LAPIC_Registers offset, uint64_t value) {
    write_msr(0x800 + offset,value);
}

uint64_t read_from_LAPIC(enum LAPIC_Registers offset) {
    return read_msr(0x800 + offset);
}

void end_of_interrupt() {
    write_to_LAPIC(EOI,0);
}

static struct Timer_waits* waiting_list;
static uint64_t waiting_count;

static struct Timer_waits* empty_list;
static uint64_t empty_count;

uint64_t messurre_LAPIC_timer(){
    //divider is 2^(n+2), 0b111 or 0x7 is divide by 1, most significant bit is offset to the left by one bit
    write_to_LAPIC(DIVIDE_CONFIGURATION_REGISTER, 0b1011);
    //PIT_start();

    write_to_LAPIC(INIT_COUNT_REGISTER, 0xFFFFFFFF);
    init_PIT();

    write_to_LAPIC(LVT_TIMER, LVT_R_MASK_INTERRUPTS);

    ticks_per_ms = 0xFFFFFFFF - read_from_LAPIC(CURRENT_COUNT_TIMER);
    uint64_t ticks = get_time();
    uint64_t busf = cpuid(0x15);
    uint64_t coref = cpuid(0x16);
    //ticks_per_ms /= get_time();
    return ticks_per_ms;
}

uint64_t convert_to_timer_ticks(uint64_t time){
    return time * ticks_per_ms;
}

//this sets the amount of ticks until the next interrupt shall be fired
void set_LAPIC_time(uint64_t LAPIC_ticks){
    write_to_LAPIC(INIT_COUNT_REGISTER,LAPIC_ticks);
}

//adds the specific process into the queue
void add_to_sleep(uint64_t delta,uint64_t tID){
    delta = convert_to_timer_ticks(delta);
    if (empty_count == 0){
        struct Timer_waits* new_waiting = malloc(sizeof(struct Timer_waits) * 10);
        memset(new_waiting,0,sizeof(struct Timer_waits) * 10);
        for (int i = 0; i < 9; i++){
            new_waiting[i].next = &new_waiting[i + 1];
        }
        empty_list = new_waiting;
        waiting_count += 10;
    }

    struct Timer_waits *new_waiting = empty_list;
    empty_list = new_waiting->next;
    empty_count--;

    new_waiting->tID = tID;

    uint64_t current_waiting = 0;
    struct Timer_waits *current = waiting_list;
    if (current == NULL){
        new_waiting->relative_time = delta;
        waiting_list = new_waiting;
        waiting_count++;
        return;
    }

    if (current->relative_time > delta){
        new_waiting->next = waiting_list;
        waiting_list = new_waiting;
        new_waiting->next->relative_time -= delta;
        new_waiting->relative_time = delta;
        set_LAPIC_time(delta);
        waiting_count++;
        return;
    }

    struct Timer_waits *prev = 0;
    while (current != NULL){
        if (current_waiting + current->relative_time > delta)break;
        current_waiting += current->relative_time;

        prev = current;
        current = current->next;
    }

    new_waiting->next = prev->next;
    prev->next = new_waiting;
    new_waiting->relative_time = delta - current_waiting;
    waiting_count++;
}

void init_APIC_timer(){
    set_APIC_Timer_Interrupt(One_Shot,49);
    write_to_LAPIC(INIT_COUNT_REGISTER,0);
    uint32_t msr_val = read_from_LAPIC(LVT_TIMER);
    msr_val &= ~LVT_R_MASK_INTERRUPTS;
    write_to_LAPIC(LVT_TIMER,msr_val);
}

void* APIC_timer_handler(struct interrupt_frame* int_frame,void* rsp){
    awake_thread(waiting_list->tID);
    struct Timer_waits *current = waiting_list;
    current->next = 0;
    current->relative_time = 0;
    current->tID = 0;
    waiting_list = waiting_list->next;
    waiting_count--;

    while (waiting_list->relative_time == 0){
        awake_thread(waiting_list->tID);
        current = waiting_list;
        current->next = 0;
        current->relative_time = 0;
        current->tID = 0;
        waiting_list = waiting_list->next;
        waiting_count--;
    }

    if (waiting_count > 0)set_LAPIC_time(waiting_list->relative_time);
    memset(current,0,sizeof(struct Timer_waits));

    return pop_next_and_readd_running(rsp)->rsp;
}
