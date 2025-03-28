//
// Created by igel on 09.11.24.
//

#include "../../include/kernel/vtty.h"
#include "kernel.h"
#include <limine.h>
#include <string.h>
#include <kernel/bitmap.h>
#include <stdbool.h>
#include <stdio.h>

#define virt_x 3840
#define virt_y 2160

static uint8_t bpp;
static void* vbuffer;
static uint64_t height;
static uint64_t width;

void video_init(struct limine_framebuffer* buffer){
    vbuffer = buffer->address;
    height = buffer->height;
    width = buffer->width;
    bpp = 32;
    video_clear(0x00000000);
}

void video_clear(uint32_t color){
    for (int i = 0; i < (height * width); ++i){
        ((uint32_t*)vbuffer)[i] = color;
    }
}

void video_image(struct video_Image* img,uint32_t x,uint32_t y,uint32_t u,uint32_t v){
    x = x * width / virt_x;
    y = y * height / virt_y;
    u = u / img->width;
    v = v / img->height;
    uint8_t state = 0;
    state |= (u == 0 && v == 0) * 1;
    state |= (u == 1 && v == 1) * 0b10;
    state |= (u > 1 && v > 0) * 0b100;

    if(state == 0)
        hcf();

    if(state == 0b10) {
        uint32_t *buffer = vbuffer + (y * width + x) * 4;

        uint64_t i = 0;
        uint8_t *pixels = (uint8_t *) img->pixels;
        uint8_t *cur = (uint8_t *) buffer;
        for (uint32_t k = 0; k < img->height; ++k) {
            if(k == 240){
                int i = 0;
            }

            for (uint32_t j = 0; j < img->width; ++j) {
                //if(k == 250)printf("%d ",j);
                /*cur[i * 4] = 255;
                cur[i * 4 + 1] = (pixels[i * 4] == 0 ? 0 : (pixels[i * 4 + 1] * 255 / pixels[i * 4])) + ((pixels[i * 4] == 255) ? 0: (cur[i * 4 + 1] * 255 / (255 - pixels[i * 4])));
                cur[i * 4 + 2] = (pixels[i * 4] == 0 ? 0 : (pixels[i * 4 + 2] * 255 / pixels[i * 4])) + ((pixels[i * 4] == 255) ? 0: (cur[i * 4 + 2] * 255 / (255 - pixels[i * 4])));
                cur[i * 4 + 3] = (pixels[i * 4] == 0 ? 0 : (pixels[i * 4 + 3] * 255 / pixels[i * 4])) + ((pixels[i * 4] == 255) ? 0: (cur[i * 4 + 3] * 255 / (255 - pixels[i * 4])));*/
                cur[i * 4] = (pixels[i * 4 + 3] == 0 ?0:pixels[i * 4] * 255 / pixels[i * 4 + 3]) + (pixels[i * 4 + 3] == 255?0:(cur[i * 4] * 255 / (255 - pixels[i * 4 + 3])));
                cur[i * 4 + 1] = (pixels[i * 4 + 3] == 0 ?0:pixels[i * 4 + 1] * 255 / pixels[i * 4 + 3]) + (pixels[i * 4 + 3] == 255?0:(cur[i * 4 + 1] * 255 / (255 - pixels[i * 4 + 3])));
                cur[i * 4 + 2] = (pixels[i * 4 + 3] == 0 ?0:pixels[i * 4 + 2] * 255 / pixels[i * 4 + 3]) + (pixels[i * 4 + 3] == 255?0:(cur[i * 4 + 2] * 255 / (255 - pixels[i * 4 + 3])));
                i++;
            }
            cur += (width - img->width) * 4; //(width - img->width * width / virt_x) * 4;
        }
        int z = 0;
    }else if(state == 0b100){
        uint32_t *buffer = vbuffer + (y * width + x) * 4;
        uint32_t* buf2 = buffer;

        uint64_t i = 0;
        uint8_t *pixels = (uint8_t *) img->pixels;
        uint8_t *cur = (uint8_t *) buffer;
        for (uint32_t k = 0; k < img->height /* * height / virt_y*/; ++k) {
            buffer = buf2;
            for (uint32_t j = 0; j < img->width /* * width / virt_x*/; ++j) {
                uint32_t * buf = buffer;
                for (uint32_t l = 0; l < u; ++l) {
                    uint32_t* b = (buf + (width * l));
                    cur = (uint8_t*) b;
                    for (uint32_t m = 0; m < v; ++m) {
                        cur[m * 4] = (pixels[i * 4 + 3] == 0 ?0:pixels[i * 4] * 255 / pixels[i * 4 + 3]) + (pixels[i * 4 + 3] == 255?0:(cur[m * 4] * 255 / (255 - pixels[i * 4 + 3])));
                        cur[m * 4 + 1] = (pixels[i * 4 + 3] == 0 ?0:pixels[i * 4 + 1] * 255 / pixels[i * 4 + 3]) + (pixels[i * 4 + 3] == 255?0:(cur[m * 4 + 1] * 255 / (255 - pixels[i * 4 + 3])));
                        cur[m * 4 + 2] = (pixels[i * 4 + 3] == 0 ?0:pixels[i * 4 + 2] * 255 / pixels[i * 4 + 3]) + (pixels[i * 4 + 3] == 255?0:(cur[m * 4 + 2] * 255 / (255 - pixels[i * 4 + 3])));
                        //cur[m * 4 + 3] = 255;
                    }
                }
                buffer = buf + u;
                i++;
            }
            buf2 += width * u; //* v - img->width * u;
            //buffer +=  //(width * v - img->width * width * u);//(width * v * img->height);
        }
    }
}

/*
for (uint32_t k = 0; k < img->height; ++k) {
            for (uint32_t j = 0; j < img->width; ++j) {
                uint32_t * buf = buffer;
                for (uint32_t l = 0; l < u; ++l) {
                    buffer = buf + (width * l);
                    for (uint32_t m = 0; m < v; ++m) {
                        buffer[m] = img->pixels[i];
                    }
                }
                buffer = buf;
                buffer += u;
                i++;
            }
            buffer += (width * v - img->width * u);
        }
 */

void video_text(struct video_Text* txt,uint32_t x,uint32_t y){
    x = x * width / virt_x;
    y = y * height / virt_y;
    uint32_t size = txt->size * (height / (virt_y / 4));
    const uint32_t*buffer = vbuffer + (y * width + x) * 4;
    uint32_t* modBuf;
    for (uint32_t k = 0; k < strlen(txt->txt); ++k) {
        char *bits = get_char_bitmap(txt->txt[k]);
        modBuf = buffer + 12 * k * size;
        for (int i = 0; i < 13; i++) {
            for (int j = 0; j < 8; j++) {
                bool b = bits[12 - i] & (0b10000000 >> j);
                uint32_t col = b * txt->color;
                for (uint32_t l = 0; l < size; ++l) {
                    modBuf[0] = col;
                    for (uint32_t m = 0; m < size; ++m){
                        modBuf[width * m] = col;
                    }
                    modBuf++;
                }
            }
            modBuf += (width - 8) * size;
        }
    }
}

void video_rect(struct video_Rect* rect,uint32_t x,uint32_t y){
    x = x * width / virt_x;
    y = y * height / virt_y;
    uint32_t *buffer = vbuffer + (y * width + x) * 4;

    uint8_t *cur = (uint8_t *) buffer;
    uint8_t* col = (uint8_t*) &rect->color;
    uint8_t s = col[3];
    uint8_t r = col[2];
    uint8_t g = col[1];
    uint8_t b = col[0];
    for(uint32_t i = 0; i < rect->height * height / virt_y;i++){
        cur = (uint8_t*) (buffer + width * i);
        for(uint32_t j = 0;j < rect->width * width / virt_x;j++){
            cur[0] = (s == 0 ?0:r * 255 / s) + (s == 255?0:(cur[0] * 255 / (255 - r)));
            cur[1] = (s == 0 ?0:g * 255 / s) + (s == 255?0:(cur[1] * 255 / (255 - g)));
            cur[2] = (s == 0 ?0:b * 255 / s) + (s == 255?0:(cur[2] * 255 / (255 - b)));
            cur += 4;
        }
    }
}

uint32_t get_res_x(){
    return width;
}

uint32_t get_res_y(){
    return height;
}