#include "include/Lexer.h"


static char* reserved[] = {
    "printf",
    "<digit>",
};


static void kc_lex_reset_buffer(lexer_t* lexer) {
    memset(lexer->buffer, '\0', strlen(lexer->buffer));
    lexer->bufferidx = 0;
    lexer->buffer = (char*)realloc(lexer->buffer, sizeof(char));
}


static void kc_lex_bufpush(lexer_t* lexer, char c) {
    lexer->buffer[lexer->bufferidx] = c;
    ++lexer->bufferidx;
}


static tokentype_t kc_lex_buffervalid(char* buffer) {
    if (strcmp(buffer, reserved[T_PRINT]) == 0) {
        return T_PRINT;
    }

    return INVLD_TOKEN;
}



tokenlist_t kc_lex_tokenize(lexer_t* lexer, char* buffer) {
    while (lexer->idx < strlen(buffer)) {
        lexer->curChar = buffer[lexer->idx];

        if (IS_WHITESPACE(lexer->curChar)) {
            printf("%s\n", lexer->buffer);
            kc_lex_reset_buffer(lexer);
            ++lexer->idx;
            continue;
        } else if (IS_NEWLINE(lexer->curChar)) {
            printf("%s\n", lexer->buffer);
            ++lexer->line;
            ++lexer->idx;
            continue;
        }


        // Buffer check.
        if (!(IS_ALPHA_LOWER(lexer->curChar)) && !(IS_ALPHA_UPPER(lexer->curChar)) && !(IS_DIGIT(lexer->curChar)) && lexer->onword) {
            printf("%s\n", lexer->buffer);
            kc_lex_reset_buffer(lexer);
            lexer->onword = false;
        } else {
            lexer->onword = true;
        }

        kc_lex_bufpush(lexer, lexer->curChar);
        ++lexer->idx;
    }

    free(lexer->buffer);
    lexer->buffer = NULL;
}
