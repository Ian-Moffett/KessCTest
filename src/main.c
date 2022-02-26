#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "include/Lexer.h"
#include "include/Token.h"


int main(int argc, char* argv[]) {
    char* srcFile = NULL;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            srcFile = argv[i];
            break;
        }
    }

    if (!(srcFile)) {
        printf("Usage: %s <optional args> <filename>\n", argv[0]);
        exit(0);
    } else if (access(srcFile, F_OK) != 0) {
        printf("Cannot access %s\n", srcFile);
        exit(1);
    }

    FILE* fp = fopen(srcFile, "r");

    fseek(fp, 0, SEEK_END);
    unsigned long long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buffer = (char*)calloc(fsize, sizeof(char));

    fread(buffer, sizeof(char), fsize, fp);

    tokenlist_t tokenlist = {
        .size = 0,
        .tokens = (token_t*)malloc(sizeof(token_t))
    };

    lexer_t lexer = {
        .line = 1,
        .idx = 0,
        .buffer = (char*)calloc(2, sizeof(char)),
        .bufferidx = 0,
        .error = false,
        .tokenlist = tokenlist,
        .onword = false,
    };

    kc_lex_tokenize(&lexer, buffer);

    destroy_tokenlist(&tokenlist);
    free(buffer);
    fclose(fp);
}
