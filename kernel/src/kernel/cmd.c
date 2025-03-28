//
// Created by igel on 11.11.24.
//

#include <kernel/cmd.h>
#include <string.h>
#include <kernel/vtty.h>
#include <kernel.h>
#include "drivers/keyboard.h"

uint8_t slide_i = 0;

#define L_ARROW 0x4D
#define R_ARROW 0x4B

#define virt_x 3840
#define virt_y 2160

uint32_t sls[][10] = {};

void handle_CMD(const char* cmd){
    /*if(strcmp(cmd,"presi") == 0){
        presentation();
    }else if(strcmp(cmd,"presi2") == 0){
        presentation2();
    }*/
}

/*void update(){
    if(slide_i < 0)slide_i = 0;
    else if(slide_i >= 29)slide_i = 28;
    struct video_Image img;
    img.width = 1280;
    img.height = 720;
    img.pixels = slides_1[slide_i];
    video_image(&img,0,0,1280,720);
}

void on_key(bool pressed,int code){
    if(!pressed)return;
    if(code == L_ARROW){
        slide_i++;
        update();
    }else if(code == R_ARROW){
        slide_i--;
        update();
    }
}

void presentation(){
    set_keyboard_handler(&on_key);
    video_init(get_FrameBuffer());
    update();
}

void update2(){
    if(slide_i < 0)slide_i = 0;
    else if(slide_i >= 29)slide_i = 28;
    struct video_Image img;
    img.width = 1280;
    img.height = 720;
    img.pixels = slides_2[slide_i];
    video_image(&img,0,0,1280,720);
}

void on_key2(bool pressed,int code){
    if(!pressed)return;
    if(code == L_ARROW){
        slide_i++;
        update2();
    }else if(code == R_ARROW){
        slide_i--;
        update2();
    }
}

void presentation2(){
    set_keyboard_handler(&on_key2);
    video_init(get_FrameBuffer());
    update2();
}*/