#include "include/CodeGen.h"

extern bool asmonly;


void kc_gen_machine_code(ast_t ast) {
    section_t curSection = S_TEXT;
    ast_node_t curNode;

    bool codegenerror = false;
    bool scoped = false;

    unsigned long long symbolCount = 0;

    for (int i = 0; i < ast.size; ++i) {
        if (strcmp(ast.nodes[i].key, "VAR") == 0) {
            ++symbolCount;
        }
    }

    const unsigned long long SYMBOLCOUNT = symbolCount;
    symbol_t symbol_table[SYMBOLCOUNT];
    symbol_t local_symbols[SYMBOLCOUNT];

    for (int i = 0; i < SYMBOLCOUNT; ++i) {
        symbol_table[i].key = NULL;
        symbol_table[i].offset = 0;
        local_symbols[i].key = NULL;
        local_symbols[i].offset = 0;
    }

    unsigned long long lcodec = 0;   // Code label count.
    unsigned long long ldatac = 0;   // Data label count.
    unsigned long long lcfsc = 0;    // Control flow end label count.
    unsigned int sfac = 0;           // Stack frame allocation count.
    bool incall = false;

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
            if (curSection != S_TEXT) {
                curSection = S_TEXT;
                fprintf(fp, "section .text\n");
            }

            fprintf(fp, "_%d: jmp _%d\n\n", lcodec, lcodec + 1);
            ++lcodec;

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

            if (!(scoped)) {
                fprintf(fp, "_%d:\n", lcodec);
            }

            fprintf(fp, "    mov eax, 4\n");
            fprintf(fp, "    mov ebx, 1\n");
            fprintf(fp, "    mov edx, %d\n", strlen(curNode.value) + 1);
            fprintf(fp, "    mov ecx, LC%d\n", ldatac - 1);
            fprintf(fp, "    int 0x80\n\n");
            ++lcodec;
        } else if (strcmp(curNode.key, "VAR") == 0) {
            if (curSection != S_TEXT && !(scoped)) {
                fprintf(fp, "section .text\n");
            }

            if (!(scoped)) {
                fprintf(fp, "_%d: jmp _%d\n\n", lcodec, lcodec + 1);
                ++lcodec;
            }

            if (curSection != S_DATA && strcmp(curNode.children[1].key, "VALUE") == 0 && !(scoped)) {
                curSection = S_DATA;
                fprintf(fp, "section .data\n");
            } else if (!(scoped)) {
                curSection = S_BSS;
                fprintf(fp, "section .bss\n");
            }

            symbol_t var;

            if (!(scoped)) {
                var.strVal = curNode.children[1].value;
                var.intVal = 0;
                var.key = curNode.value;
            } else {
                var.key = curNode.value;
                var.intVal = 0;
                var.key = curNode.value; 
            }

            symbol_t* symb;
            
            if (!(scoped)) {
                symb = &symbol_table[symb_tbl_hash(curNode.value, SYMBOLCOUNT)];  
            } else {
                symb = &symbol_table[symb_tbl_hash(curNode.value, SYMBOLCOUNT)];  
            }

            if (symb->key != NULL) {
                if (strcmp(symb->key, curNode.value) == 0) {
                    kc_log_err("SymbolError: Trying to redefine symbol.", curNode.value, curNode.lineNumber);
                    codegenerror = true;
                    break;
                }
            }


            if (strcmp(curNode.children[0].value, "uint8") == 0 && curSection == S_BSS) { 
                fprintf(fp, "v_%s: resb 1\n\n", curNode.value);
            } else if (curSection == S_DATA) {
                if (strcmp(curNode.children[0].key, "uint8") == 0) {
                    var.intVal = atoi(curNode.children[1].value);
                }
                
                fprintf(fp, "v_%s: db %s\n\n", curNode.value, curNode.children[1].value);
            } else if (strcmp(curNode.children[0].value, "uint8") == 0 && scoped) {
                fprintf(fp, "    push %s\n\n", curNode.children[1].value);
                fprintf(fp, "    section .data\n");
                fprintf(fp, "    .vs_%s: dd 0x0\n\n", curNode.value);
                fprintf(fp, "    section .text\n");
                fprintf(fp, "    mov [.vs_%s], esp\n\n", curNode.value);
            }
 
            *symb = var;
        } else if (strcmp(curNode.key, "PRINTF_VAR") == 0) {
            if (!(scoped) && !(symbol_table[symb_tbl_hash(curNode.value, SYMBOLCOUNT)].key)) {                
                kc_log_err("SymbolError: Symbol does not exist.", curNode.value, curNode.lineNumber);
                codegenerror = true;
                break;
            } else if (scoped && !(symbol_table[symb_tbl_hash(curNode.value, SYMBOLCOUNT)].key)) {
                kc_log_err("SymbolError: Symbol does not exist.", curNode.value, curNode.lineNumber);
                codegenerror = true;
                break;
            }
            
            if (curSection != S_TEXT && !(scoped)) {
                fprintf(fp, "section .text\n");
                curSection = S_TEXT;
            }

            if (!(scoped)) {
                fprintf(fp, "_%d: jmp _%d\n\n",  lcodec, lcodec + 1);
                ++lcodec;
            }

            if (curSection != S_RODATA && !(scoped)) {
                fprintf(fp, "section .rodata\n");
                curSection = S_RODATA;
            }

            // TODO: Allow expressions like this: printf(var + 1);
            if (strcmp(curNode.children[0].value, "TRUE")) {
                if (!(scoped)) {
                    fprintf(fp, "LC%d: db \"%s\", 0xA\n\n", ldatac, symbol_table[symb_tbl_hash(curNode.value, SYMBOLCOUNT)].strVal);
                    ++ldatac;
                }

                if (!(scoped)) {
                    curSection = S_TEXT;
                    fprintf(fp, "section .text\n");
                }

                if (!(scoped)) {
                    fprintf(fp, "_%d:\n", lcodec);
                }

                fprintf(fp, "    mov eax, 4\n");
                fprintf(fp, "    mov ebx, 1\n");
                fprintf(fp, "    mov edx, %d\n", strlen(symbol_table[symb_tbl_hash(curNode.value, SYMBOLCOUNT)].strVal) + 1);

                if (!(scoped)) {
                    fprintf(fp, "    mov ecx, LC%d\n", ldatac - 1);
                } else {
                    fprintf(fp, "    mov ecx, [.vs_%s]\n", curNode.value);
                }

                fprintf(fp, "    int 0x80\n\n");
                ++lcodec;
            }
        } else if (strcmp(curNode.key, "IF") == 0) {
            scoped = true;

            if (strcmp(curNode.children[0].value, "TRUE") == 0) {
                if (!(symbol_table[symb_tbl_hash(curNode.value, SYMBOLCOUNT)].key) || SYMBOLCOUNT == 0) {             
                    kc_log_err("SymbolError: Symbol does not exist.", curNode.value, curNode.lineNumber);
                    codegenerror = true;
                    break;
                }
            }

            if (strcmp(curNode.children[3].value, "TRUE") == 0) {
                if (!(symbol_table[symb_tbl_hash(curNode.children[2].value, SYMBOLCOUNT)].key) || SYMBOLCOUNT == 0) {
                    kc_log_err("SymbolError: Symbol does not exist.", curNode.children[2].value, curNode.lineNumber);
                    codegenerror = true;
                    break;
                } 
            }

            if (curSection != S_TEXT) {
                fprintf(fp, "section .text\n");
                curSection = S_TEXT;
            }


            
            fprintf(fp, "; if (%s %s %s)\n", curNode.value, curNode.children[1].value, curNode.children[2].value);
            fprintf(fp, "_%d:\n", lcodec);
            ++lcodec;
            // Setup stack frame.
            fprintf(fp, "    ; Stack frame.\n");
            fprintf(fp, "    push ebp\n");
            fprintf(fp, "    mov ebp, esp\n");
            fprintf(fp, "    sub esp, 0x10\n\n");

            if (strcmp(curNode.children[1].value, "==") == 0) {
                if (strcmp(curNode.children[0].value, "TRUE") != 0) {
                    fprintf(fp, "    ; Comparing values.\n");
                    fprintf(fp, "    mov ebx, %s\n", curNode.value);
                    fprintf(fp, "    cmp ebx, %s\n", curNode.children[2].value);
                    fprintf(fp, "    ; If EBX != %s then skip code in if statement.\n", curNode.value);
                    fprintf(fp, "    jne CFS%d\n\n", lcfsc);
                }
            }
        } else if (strcmp(curNode.key, "SCOPE-END") == 0) { 
            if (curSection != S_TEXT) {
                curSection = S_TEXT;
                fprintf(fp, "section .text\n");
            }


            fprintf(fp, "; Destroys stack frame.\n");
            fprintf(fp, "_%d:\n", lcodec);
            ++lcodec;

            // Destroy stack frame.
            
            for (int i = 0; i < sfac; ++i) {
                fprintf(fp, "    pop ebx\n");
            }

            fprintf(fp, "    pop ebp\n\n");

            if (incall) {
                fprintf(fp, "    ret\n");
            } else {
                fprintf(fp, "CFS%d: jmp _%d\n\n", lcfsc, lcodec);
                ++lcfsc;
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
