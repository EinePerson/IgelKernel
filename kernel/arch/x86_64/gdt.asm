global setGdt
global reloadSegments
global setTSS

section .data

gdtr DW 0 ; For limit storage
     DQ 0 ; For base storage

section .text

setGdt:
   MOV   [gdtr], DI
   MOV   [gdtr+2], RSI
   LGDT  [gdtr]
   RET

setTSS:
    CLI
    MOV AX,0x28
    ltr AX
    RET

reloadSegments:
    CLI
   ; Reload CS register:
   PUSH 0x08 ; Push code segment to stack, 0x08 is a stand-in for your code segment
   LEA RAX, [rel .reload_CS] ; Load address of .reload_CS into RAX
   PUSH RAX                  ; Push this value to the stack
   RETFQ                     ; Perform a far return, RETFQ or LRETQ depending on syntax
.reload_CS:
   ; Reload data segment registers
   MOV   AX, 0x10 ; 0x10 is a stand-in for your data segment
   MOV   DS, AX
   MOV   ES, AX
   MOV   FS, AX
   MOV   GS, AX
   MOV   SS, AX
   RET