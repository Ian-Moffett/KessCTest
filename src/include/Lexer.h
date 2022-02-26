#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>
#include <stdio.h>


typedef struct {
    unsigned long long line;
    unsigned long long row;
    unsigned char curChar;
    char* buffer;
} lexer_t;


#endif
