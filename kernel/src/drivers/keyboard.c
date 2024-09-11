//
// Created by igel on 06.09.24.
//

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <util.h>
#include <drivers/keyboard.h>

#define DATA_PORT 0x60
#define CMD_PORT 0x64

const uint32_t UNKNOWN = 0xFFFFFFFF;
const uint32_t ESC = 0xFFFFFFFF - 1;
const uint32_t CTRL = 0xFFFFFFFF - 2;
const uint32_t LSHFT = 0xFFFFFFFF - 3;
const uint32_t RSHFT = 0xFFFFFFFF - 4;
const uint32_t ALT = 0xFFFFFFFF - 5;
const uint32_t F1 = 0xFFFFFFFF - 6;
const uint32_t F2 = 0xFFFFFFFF - 7;
const uint32_t F3 = 0xFFFFFFFF - 8;
const uint32_t F4 = 0xFFFFFFFF - 9;
const uint32_t F5 = 0xFFFFFFFF - 10;
const uint32_t F6 = 0xFFFFFFFF - 11;
const uint32_t F7 = 0xFFFFFFFF - 12;
const uint32_t F8 = 0xFFFFFFFF - 13;
const uint32_t F9 = 0xFFFFFFFF - 14;
const uint32_t F10 = 0xFFFFFFFF - 15;
const uint32_t F11 = 0xFFFFFFFF - 16;
const uint32_t F12 = 0xFFFFFFFF - 17;
const uint32_t SCRLCK = 0xFFFFFFFF - 18;
const uint32_t HOME = 0xFFFFFFFF - 19;
const uint32_t UP = 0xFFFFFFFF - 20;
const uint32_t LEFT = 0xFFFFFFFF - 21;
const uint32_t RIGHT = 0xFFFFFFFF - 22;
const uint32_t DOWN = 0xFFFFFFFF - 23;
const uint32_t PGUP = 0xFFFFFFFF - 24;
const uint32_t PGDOWN = 0xFFFFFFFF - 25;
const uint32_t END = 0xFFFFFFFF - 26;
const uint32_t INS = 0xFFFFFFFF - 27;
const uint32_t DEL = 0xFFFFFFFF - 28;
const uint32_t CAPS = 0xFFFFFFFF - 29;
const uint32_t NONE = 0xFFFFFFFF - 30;
const uint32_t ALTGR = 0xFFFFFFFF - 31;
const uint32_t NUMLCK = 0xFFFFFFFF - 32;


const uint32_t lowercase[128] = {
    UNKNOWN,ESC,'1','2','3','4','5','6','7','8',
    '9','0','-','=','\b','\t','q','w','e','r',
    't','y','u','i','o','p','[',']','\n',CTRL,
    'a','s','d','f','g','h','j','k','l',';',
    '\'','`',LSHFT,'\\','z','x','c','v','b','n','m',',',
    '.','/',RSHFT,'*',ALT,' ',CAPS,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,NUMLCK,SCRLCK,HOME,UP,PGUP,'-',LEFT,UNKNOWN,RIGHT,
    '+',END,DOWN,PGDOWN,INS,DEL,UNKNOWN,UNKNOWN,UNKNOWN,F11,F12,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    };



static bool both_supported = false;

static bool first_connected = false;
static uint8_t first_type;
static bool second_connected = false;
static uint8_t second_type;

void init_keyboard() {
    /*wait();
    send(0xAD,0,0);
    send(0xA7,0,0);
    inb(DATA_PORT);
    wait();

    //Clear IRQ and translation enable clock line
    send(0x20,0,0);
    uint8_t ccb = inb(DATA_PORT);
    ccb &= 0b10101110;
    send(0x60,&ccb,1);
    send(0xAA,0,0);
    wait();
    uint8_t test = inb(DATA_PORT);
    if(test != 0x55) {
        printf("Keyboard test failed");
        abort();
    }

    //check wether 2 PS/2 controllers are present
    send(0xA8,0,0);
    send(0x20,0,0);
    uint8_t second_test = inb(DATA_PORT);
    both_supported = !(second_test & 0b00100000 > 0);
    if(!both_supported) {
        printf("Second PS/2 port unsupported");
        abort();
    }
    send(0xA7,0,0);


    second_test &= 0b11011101;
    outb(CMD_PORT,0x60);
    outb(DATA_PORT,second_test);

    //Check weather the PS/2 ports are working
    send(0xAB,0,0);
    if(inb(DATA_PORT)) {
        printf("First PS/2 port broken");
        abort();
    }

    send(0xA9,0,0);
    if(inb(DATA_PORT)) {
        printf("Second PS/2 port broken");
        abort();
    }

    */
    both_supported = true;
    //Enable PS/2 devices
    /*send(0xAE,0,0);
    send(0xA8,0,0);
    send(0x20,0,0);
    uint8_t ccb = inb(DATA_PORT);
    ccb |= 0b11;
    send(0x60,&ccb,1);

    send_first(0xF6,0,0);
    send_second(0xF6,0,0);*/

    /*outb(DATA_PORT,0);
    uint8_t first = send_second(0xFF,0,0);
    if(first != 0xAA && first != 0xFA) {
        first_connected = false;
    }else {
        first_connected = true;
        uint8_t scan_code_set = 2;
        //uint8_t resp = send_first(0xF0,&scan_code_set,1);
        //resp = send_first(0xF4,0,0);
    }

    outb(DATA_PORT,0);
    uint8_t second = send_second(0xFF,0,0);
    if(second != 0xAA && second != 0xFA) {
        second_connected = false;
    }else {
        second_connected = true;
        uint8_t scan_code_set = 2;
        uint8_t resp = send_first(0xF0,&scan_code_set,1);
        resp = send_first(0xF4,0,0);
    }*/

    /*uint8_t scan_code_set = 2;
    uint8_t resp = send_first(0xF0,&scan_code_set,1);
    scan_code_set = 0;
    resp = send_second(0xF0,&scan_code_set,1);
    wait();
    resp = read();*/

    //resp = send_second(0xF0,&scan_code_set,1);
}


uint8_t send(unsigned char cmd,unsigned char *data, uint8_t data_length) {
    unsigned char response = 0xFE;
    char tries = 0;
    while (response == (unsigned char) 0xFE) {
        wait();
        outb(DATA_PORT,0);
        outb(CMD_PORT,cmd);
        for(uint8_t i = 0;i < data_length;i++) {
            wait();
            outb(DATA_PORT,data[i]);
        }
        response = inb(DATA_PORT);
        if(response == 0xFA)return 0xFA;
        if(response == 0xAA)return 0xAA;
        if(response == 0xFE)tries++;
        if(tries >= 3) {
            printf("Error in PS/2 device");
            abort();
        }
    }
    return response;
}

uint8_t send_first(unsigned char cmd, unsigned char *data, uint8_t data_length) {
    if(!both_supported) {
        printf("Second PS/2 device is unsupported but interacted with");
        abort();
    }

    unsigned char response = 0xFE;
    char tries = 0;
    while (response == (unsigned char) 0xFE) {
        wait();
        outb(DATA_PORT,cmd);
        for(uint8_t i = 0;i < data_length;i++) {
            wait();
            outb(DATA_PORT,data[i]);
        }
        response = inb(DATA_PORT);
        outb(DATA_PORT,0);
        if(response == 0xFA)return 0xFA;
        if(response == 0xAA)return 0xAA;
        if(response == 0xFE)tries++;
        if(tries >= 3){
            printf("Error in second PS/2 device");
            abort();
        }
    }
    return response;
}

uint8_t send_second(unsigned char cmd, unsigned char *data, uint8_t data_length) {
    if(!both_supported) {
        printf("Second PS/2 device is unsupported but interacted with");
        abort();
    }

    unsigned char response = 0xFE;
    char tries = 0;
    while (response == (unsigned char) 0xFE) {
        wait();
        outb(CMD_PORT,0xD4);
        outb(DATA_PORT,cmd);
        for(uint8_t i = 0;i < data_length;i++) {
            wait();
            outb(DATA_PORT,data[i]);
        }
        response = inb(DATA_PORT);
        outb(DATA_PORT,0);
        if(response == 0xFA)return 0xFA;
        if(response == 0xAA)return 0xAA;
        if(response == 0xFE)tries++;
        if(tries >= 3){
            printf("Error in second PS/2 device");
            abort();
        }
    }
    return response;
}

uint8_t read() {
    wait();
    uint8_t ret = inb(DATA_PORT);
    wait();
    outb(DATA_PORT,0);
    return ret;
}

void end_of_interrupt();

__attribute__ ((interrupt))
void keyboard_handler(struct interrupt_frame *frame) {
    char code = inb(DATA_PORT) & 0x7F;
    char pressed = inb(DATA_PORT) & 0x80;

    if (pressed == 0 && code != 42){
        printf("%c", lowercase[code]);
    }


    end_of_interrupt();
}

void wait() {
    while (inb(CMD_PORT) & 0x02) {

    }
}
