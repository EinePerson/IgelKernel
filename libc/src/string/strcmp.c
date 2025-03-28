//
// Created by igel on 11.11.24.
//

#include <string.h>

int strcmp(const char* a,const char* b){
    int aL = strlen(a);
    int bL = strlen(b);
    if(aL != bL){
        return aL - bL;
    }

    char c = 0;
    char d = 0;
    while (c == d){
        c = *a++;
        d = *b++;
        if(c == '\0')return c - d;
    }
    return c- d;
}