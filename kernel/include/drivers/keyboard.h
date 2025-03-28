//
// Created by igel on 06.09.24.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>
#include <kernel/interrupts.h>

#define BCKSPC_Key 0xE
#define ENTR_Key 0x1C

void init_keyboard(void (*_handler)(bool,int));
void set_keyboard_handler(void (*_handler)(bool, int));

uint8_t send(unsigned char cmd,unsigned char* data,uint8_t data_length);
uint8_t send_first(unsigned char cmd,unsigned char* data,uint8_t data_length);
uint8_t send_second(unsigned char cmd,unsigned char* data,uint8_t data_length);
uint8_t read();

void keyboard_handler(struct interrupt_frame *frame);

char char_ofCode(int code);

void wait();

#endif //KEYBOARD_H
