global init_PIT
global PIT_start
global asm_PITI
global get_time

extern end_of_interrupt
extern APIC_measure_finish

section .bss
system_timer_fractions:  resd 1          ; Fractions of 1 ms since timer initialized
system_timer_ms:         resd 1          ; Number of whole ms since timer initialized
IRQ0_fractions:          resd 1          ; Fractions of 1 ms between IRQs
IRQ0_ms:                 resd 1          ; Number of whole ms between IRQs
IRQ0_frequency:          resd 1          ; Actual frequency of PIT
PIT_reload_value:        resw 1          ; Current PIT reload value
PIT_wait:                resb 0          ; control weather PIT should be active
section .text

 ;Input
 ; ebx   Desired PIT frequency in Hz

 init_PIT:
    push rax
    push rbx
    push rdx

    ; Do some checking

    mov eax,0x10000                   ;eax = reload value for slowest possible frequency (65536)
    cmp ebx,18                        ;Is the requested frequency too low?
    jbe .gotReloadValue               ; yes, use slowest possible frequency

    mov eax,1                         ;ax = reload value for fastest possible frequency (1)
    cmp ebx,1193181                   ;Is the requested frequency too high?
    jae .gotReloadValue               ; yes, use fastest possible frequency

    ; Calculate the reload value

    mov eax,3579545
    mov edx,0                         ;edx:eax = 3579545
    div ebx                           ;eax = 3579545 / frequency, edx = remainder
    cmp edx,3579545 / 2               ;Is the remainder more than half?
    jb .l1                            ; no, round down
    inc eax                           ; yes, round up
 .l1:
    mov ebx,3
    mov edx,0                         ;edx:eax = 3579545 * 256 / frequency
    div ebx                           ;eax = (3579545 * 256 / 3 * 256) / frequency
    cmp edx,3 / 2                     ;Is the remainder more than half?
    jb .l2                            ; no, round down
    inc eax                           ; yes, round up
 .l2:


 ; Store the reload value and calculate the actual frequency

 .gotReloadValue:
    push rax                          ;Store reload_value for later
    mov [PIT_reload_value],ax         ;Store the reload value for later
    mov ebx,eax                       ;ebx = reload value

    mov eax,3579545
    mov edx,0                         ;edx:eax = 3579545
    div ebx                           ;eax = 3579545 / reload_value, edx = remainder
    cmp edx,3579545 / 2               ;Is the remainder more than half?
    jb .l3                            ; no, round down
    inc eax                           ; yes, round up
 .l3:
    mov ebx,3
    mov edx,0                         ;edx:eax = 3579545 / reload_value
    div ebx                           ;eax = (3579545 / 3) / frequency
    cmp edx,3 / 2                     ;Is the remainder more than half?
    jb .l4                            ; no, round down
    inc eax                           ; yes, round up
 .l4:
    mov [IRQ0_frequency],eax          ;Store the actual frequency for displaying later


 ; Calculate the amount of time between IRQs in 32.32 fixed point
 ;
 ; Note: The basic formula is:
 ;           time in ms = reload_value / (3579545 / 3) * 1000
 ;       This can be rearranged in the following way:
 ;           time in ms = reload_value * 3000 / 3579545
 ;           time in ms = reload_value * 3000 / 3579545 * (2^42)/(2^42)
 ;           time in ms = reload_value * 3000 * (2^42) / 3579545 / (2^42)
 ;           time in ms * 2^32 = reload_value * 3000 * (2^42) / 3579545 / (2^42) * (2^32)
 ;           time in ms * 2^32 = reload_value * 3000 * (2^42) / 3579545 / (2^10)

    pop rbx                           ;ebx = reload_value
    mov eax,0xDBB3A062                ;eax = 3000 * (2^42) / 3579545
    mul ebx                           ;edx:eax = reload_value * 3000 * (2^42) / 3579545
    shrd eax,edx,10
    shr edx,10                        ;edx:eax = reload_value * 3000 * (2^42) / 3579545 / (2^10)

    mov [IRQ0_ms],edx                 ;Set whole ms between IRQs
    mov [IRQ0_fractions],eax          ;Set fractions of 1 ms between IRQs


 ; Program the PIT channel

    pushfq
    cli                               ;Disabled interrupts (just in case)

    mov al,00110100b                  ;channel 0, lobyte/hibyte, rate generator
    out 0x43, al

    mov ax,[PIT_reload_value]         ;ax = 16 bit reload value
    out 0x40,al                       ;Set low byte of PIT reload value
    mov al,ah                         ;ax = high 8 bits of reload value
    out 0x40,al                       ;Set high byte of PIT reload value

    MOV AL,1
    mov [PIT_wait],AL

    popfq

    pop rdx
    pop rbx
    pop rax

    ret

get_time:
    xor RAX,RAX
    mov EAX,[IRQ0_ms]
    RET

asm_PITI:
    push rax
    MOV AL,[PIT_wait]
    test AL,AL
    JNZ .cont
    POP RAX
    IRETQ
.cont:
    MOV AL,0
    mov [PIT_wait],AL
    call end_of_interrupt
    POP RAX
    ;the interrupt frame is disregarded and overridden to get to the instruction pointer from the previous call
    ADD RSP,0x30
    RET

    ;begin_interrupt
    ;call PITI
    ;end_interrupt
    ;NOP

PIT_start:
    PUSH RAX
    MOV AL,1
    mov [PIT_wait],AL
    POP RAX
    hlt
    ;ret