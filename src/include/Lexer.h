#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "Token.h"
#include "Debugger.h"


typedef struct {
    unsigned long long line;
    unsigned long long idx; 
    unsigned char curChar;
    char* buffer;               // Buffer for keyword matching.
    unsigned long bufferidx;
    bool error;                 // Error flag.    
    bool onword;
    tokenlist_t tokenlist;
} lexer_t;


tokenlist_t kc_lex_tokenize(lexer_t* lexer, char* buffer);

#endif
