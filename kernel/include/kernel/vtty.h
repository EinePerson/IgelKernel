//
// Created by igel on 09.11.24.
//

#include <stdint.h>

#ifndef IGELKERNEL_VTTY_H
#define IGELKERNEL_VTTY_H

struct video_Image{
    uint32_t* pixels;
    uint32_t width;
    uint32_t height;
};

void video_image(struct video_Image* img,uint32_t x,uint32_t y,uint32_t u,uint32_t v);

struct video_Text{
    char* txt;
    uint32_t color;
    uint32_t size;
};

void video_text(struct video_Text* txt,uint32_t x,uint32_t y);

void draw_rect(uint32_t color,uint32_t x,uint32_t y,uint32_t* frameBuffer);

struct video_Rect{
    uint32_t color;
    uint32_t width;
    uint32_t height;
};

void video_rect(struct video_Rect* rect,uint32_t x,uint32_t y);

void video_clear(uint32_t color);

struct limine_framebuffer;

void video_init(struct limine_framebuffer* buffer);

inline uint32_t toPixel(uint8_t a,uint8_t r,uint8_t g,uint8_t b){
    return (a << 24) | (r << 16) | (g << 8) | b;
}

uint32_t get_res_x();
uint32_t get_res_y();

#endif //IGELKERNEL_VTTY_H
