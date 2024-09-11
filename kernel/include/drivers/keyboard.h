//
// Created by igel on 06.09.24.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>
#include <kernel/interrupts.h>

void init_keyboard();

uint8_t send(unsigned char cmd,unsigned char* data,uint8_t data_length);
uint8_t send_first(unsigned char cmd,unsigned char* data,uint8_t data_length);
uint8_t send_second(unsigned char cmd,unsigned char* data,uint8_t data_length);
uint8_t read();

void keyboard_handler(struct interrupt_frame *frame);

void wait();

#endif //KEYBOARD_H
