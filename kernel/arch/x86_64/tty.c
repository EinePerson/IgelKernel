#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limine.h>

#include <kernel/tty.h>
#include <kernel/bitmap.h>

/*
THIS ASSUMES THAT THE BPP IS 32 bits

 */

static struct limine_framebuffer* frame_buffer;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t bpp;
static void* terminal_buffer;
static volatile uint32_t color;

void terminal_initialize(struct limine_framebuffer* buffer) {
	frame_buffer = buffer;
	terminal_row = 0;
	terminal_column = 0;
	terminal_buffer = frame_buffer->address;
	bpp = 32;
	terminal_clear();
	color = 0x00FFFFFF;
}

void terminal_clear(){
	memset(terminal_buffer,0x00,frame_buffer->height * frame_buffer->width * (bpp / 8));
}

void terminal_putentryat(unsigned char c, uint32_t color, size_t x, size_t y) {
	char* bits = get_char_bitmap(c);
	uint32_t* buffer = terminal_buffer + (x * frame_buffer->width + y) * 4;
	for(int i = 0;i < 13;i++){
		for(int j = 0;j < 8;j++){
			bool b = bits[12 - i] & (0b10000000 >> j);
			buffer[0] = b * color;
			buffer++;
		}
		buffer += frame_buffer->width - 8;
	}
}

void terminal_putchar(char c) {
	unsigned char uc = c;
	terminal_putentryat(uc, color, terminal_column, terminal_row);
	terminal_row += 12;
	if (terminal_column == frame_buffer->width) {
		terminal_row = 0;
		terminal_column += 16;
		scrollOverflow();
		if (terminal_row == frame_buffer->height)terminal_row = 0;
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++){
		if(data[i] == '\n'){
			terminal_row = 0;
			terminal_column += 16;
			scrollOverflow();
		}else terminal_putchar(data[i]);
	}
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

void set_text_color(uint32_t new_color){
	color = new_color;
}

void scroll(){
	terminal_column -= 16;
	memmove(terminal_buffer,terminal_buffer + frame_buffer->width * 4 * 16,frame_buffer->width * (frame_buffer->height - 16) * 4);
}

void scrollOverflow(){
	if(terminal_column >= frame_buffer->height)scroll();
}
