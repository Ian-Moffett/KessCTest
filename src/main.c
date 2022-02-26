#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include "include/Lexer.h"
#include "include/Token.h"
#include "include/Parser.h"


#if defined(_WIN32) || defined(__WIN32__) || defined(__NT__) \
    || defined (_WIN64) || defined(__WIN64__) || defined(__APPLE__)

#error          // We currently only support Linux.
#endif


int main(int argc, char* argv[]) {
    char* srcFile = NULL;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            srcFile = argv[i];
            break;
        }
    }

    if (!(srcFile)) {
        printf("Usage: %s <args> <filename>\n", argv[0]);
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

    if (lexer.error) {
        destroy_tokenlist(&lexer.tokenlist);
        free(buffer);
        fclose(fp);
        exit(1);
    }

    parser_t parser = {
        .idx = 0,
        .tokenlist = lexer.tokenlist,
        .error = false,
    };

    parse(&parser);

    if (parser.error) {
        ast_destroy(&parser.ast);
        destroy_tokenlist(&lexer.tokenlist);
        free(buffer);
        fclose(fp);
        exit(1); 
    }


    ast_destroy(&parser.ast);
    destroy_tokenlist(&lexer.tokenlist);
    free(buffer);
    fclose(fp);
}
