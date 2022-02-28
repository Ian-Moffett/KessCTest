#include "include/CodeGen.h"

extern bool asmonly;


void kc_gen_machine_code(ast_t ast) {
    section_t curSection = S_TEXT;
    ast_node_t curNode;

    bool codegenerror = false;

    unsigned long long symbolCount = 0;

    for (int i = 0; i < ast.size; ++i) {
        if (strcmp(ast.nodes[i].key, "PRINTF") == 0) {
            ++symbolCount;
        }
    }

    const unsigned long long SYMBOLCOUNT = symbolCount;
    symbol_t symbol_table[SYMBOLCOUNT];

    unsigned long long lcodec = 0;
    unsigned long long ldatac = 0;

    FILE* fp = fopen("/tmp/__KC_SOURCE.s", "w");

    fprintf(fp, "; This assembly file was generated\n"
                "; automatically by the KessC compiler.\n"
                "; KessC; made by Ian Marco Moffett\n\n");

    fprintf(fp, "section .text\n");
    fprintf(fp, "global _start\n\n");
    fprintf(fp, "_start: jmp _%d\n\n", lcodec);

    for (int i = 0; i < ast.size; ++i) {
        curNode = ast.nodes[i];

        if (strcmp(curNode.key, "PRINTF") == 0) {
            symbol_t printSym = {
                .strVal = curNode.value,
                .intVal = atoi(curNode.value),
                .key = NULL
            };

            if (curSection != S_RODATA) {
                fprintf(fp, "section .rodata\n");
                curSection = S_RODATA;
            }

            fprintf(fp, "LC%d: db \"%s\", 0xA\n\n", ldatac, curNode.value);
            ++ldatac;

            if (curSection != S_TEXT) {
                fprintf(fp, "section .text\n");
                curSection = S_TEXT;
            }

            fprintf(fp, "_%d:\n", lcodec);
            fprintf(fp, "    mov eax, 4\n");
            fprintf(fp, "    mov ebx, 1\n");
            fprintf(fp, "    mov edx, %d\n", strlen(curNode.value) + 1);
            fprintf(fp, "    mov ecx, LC%d\n", ldatac - 1);
            fprintf(fp, "    int 0x80\n\n");
            ++lcodec;
        } else if (strcmp(curNode.key, "VAR") == 0) {
            if (curSection != S_TEXT) {
                fprintf(fp, "section .text\n");
            }

            fprintf(fp, "_%d: jmp _%d\n\n", lcodec, lcodec + 1);
            ++lcodec;

            if (curSection != S_BSS && strcmp(curNode.children[1].key, "NO_INIT") == 0) {
                curSection = S_BSS;
                fprintf(fp, "section .bss\n");
            } else if (curSection != S_DATA && strcmp(curNode.children[1].key, "VALUE") == 0) {
                curSection = S_DATA;
                fprintf(fp, "section .data\n");
            }

            if (strcmp(curNode.children[0].value, "uint8") == 0 && curSection == S_BSS) {
                fprintf(fp, "v_%s: resb 1\n\n", curNode.value);
            } else if (curSection == S_DATA) {
                fprintf(fp, "v_%s: db %s\n\n", curNode.value, curNode.children[1].value);
            }
        }
    }

    if (curSection != S_TEXT) {
        fprintf(fp, "section .text\n");
    }

    fprintf(fp, "_%d:\n", lcodec);
    fprintf(fp, "    mov eax, 1\n");
    fprintf(fp, "    mov ebx, 0\n");
    fprintf(fp, "    int 0x80\n");

    fclose(fp);
    
    if (!(codegenerror) && !(asmonly)) {
        system("nasm -felf32 /tmp/__KC_SOURCE.s -o /tmp/__KC_OBJ.o");
        system("ld /tmp/__KC_OBJ.o -melf_i386 -o $PWD/a.out");
    } else if (asmonly) { 
        system("mv /tmp/__KC_SOURCE.s $PWD/KessC.out.s");
    }

    if (access("/tmp/__KL_SOURCE.s", F_OK) == 0) {
        remove("/tmp/__KL_SOURCE.s");
    }

    if (!(codegenerror)) {
        remove("/tmp/__KC_SOURCE.s");
        remove("/tmp/__KC_OBJ.o");
    }
}
