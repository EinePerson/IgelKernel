#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>
#include <stdint.h>

struct limine_framebuffer;

void terminal_initialize(struct limine_framebuffer* buffer);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_clear();
void set_text_color(uint32_t color);
void scroll();
void scrollOverflow();

#endif
