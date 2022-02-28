#ifndef CODEGEN_H
#define CODEGEN_H

#include "AST.h"
#include "Symbol.h"
#include "Debugger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


typedef enum {
    S_RODATA,
    S_DATA,
    S_BSS,
    S_TEXT,
} section_t;


void kc_gen_machine_code(ast_t ast);


#endif
