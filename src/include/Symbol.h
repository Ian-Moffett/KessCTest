#ifndef SYMBOL_H
#define SYMBOL_H

#include <string.h>

typedef struct {
    char* strVal;
    unsigned long long intVal;
    char* key;
    unsigned int offset;
} symbol_t;


unsigned long long symb_tbl_hash(const char* key, const int MAX_SIZE);


#endif
