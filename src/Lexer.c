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


static tokentype_t kc_lex_buffervalid(lexer_t* lexer) {
    if (strcmp(lexer->buffer, reserved[T_PRINT]) == 0) {
        push_token(&lexer->tokenlist, create_token(T_PRINT, reserved[T_PRINT], false));
        return T_PRINT;
    }

    return INVLD_TOKEN;
}


static bool kc_lex_iswhitespace(char* s) {
    for (int i = 0; i < strlen(s); ++i) {
        if (!(IS_WHITESPACE(s[i]))) {
            return false;
        }
    }

    return true;
}


static char* kc_lex_get_str(lexer_t* lexer, char* buffer) {
    char* strbuf = (char*)calloc(2, sizeof(char));
    unsigned long long sbidx = 0;

    while (lexer->idx < strlen(buffer)) {
        lexer->curChar = buffer[lexer->idx];

        // Escape character.
        if (lexer->curChar == '\\') {
            strbuf[sbidx] = buffer[lexer->idx + 1];
            lexer->idx += 2;
            ++sbidx;
            strbuf = (char*)realloc(strbuf, sizeof(char) * (sbidx + 2));
            continue;
        } else if (lexer->curChar == '"') {
            break;
        } else if (lexer->curChar == '\n') {
            kl_log_err("SyntaxError: Expected '\"' before EOL.", "", lexer->line);
            free(strbuf);
            return NULL;
        }

        strbuf[sbidx] = lexer->curChar;
        ++lexer->curChar;
        ++sbidx;
        strbuf = (char*)realloc(strbuf, sizeof(char) * (sbidx + 2));
        ++lexer->idx;
    }

    return strbuf;
}



tokenlist_t kc_lex_tokenize(lexer_t* lexer, char* buffer) {
    while (lexer->idx < strlen(buffer) && !(lexer->error)) {
        lexer->curChar = buffer[lexer->idx];

        if (IS_WHITESPACE(lexer->curChar)) {
            if (kc_lex_buffervalid(lexer) == INVLD_TOKEN) {
                kl_log_err("TokenError: Invalid token found while lexing.", lexer->buffer, lexer->line);
                lexer->error = true;
                break;
            }

            kc_lex_reset_buffer(lexer);
            ++lexer->idx;
            continue;
        } else if (IS_NEWLINE(lexer->curChar) || lexer->curChar == ';') {
            if (kc_lex_buffervalid(lexer) == INVLD_TOKEN && !(kc_lex_iswhitespace(lexer->buffer))) {
                kl_log_err("TokenError: Invalid token found while lexing.", lexer->buffer, lexer->line);
                lexer->error = true;
                break;
            }
            
            kc_lex_reset_buffer(lexer);
            ++lexer->line;
            ++lexer->idx;
            continue;
        }


        // Buffer check.
        if (!(IS_ALPHA_LOWER(lexer->curChar)) && !(IS_ALPHA_UPPER(lexer->curChar)) && !(IS_DIGIT(lexer->curChar)) && lexer->onword) {
            if (kc_lex_buffervalid(lexer) == INVLD_TOKEN && !(kc_lex_iswhitespace(lexer->buffer))) {
                kl_log_err("TokenError: Invalid token found while lexing.", lexer->buffer, lexer->line);
                lexer->error = true;
                break;
            }

            kc_lex_reset_buffer(lexer);
            lexer->onword = false;
        } else {
            lexer->onword = true;
        }

        switch (lexer->curChar) {
            case '(':
                push_token(&lexer->tokenlist, create_token(T_LPAREN, "(", false));
                ++lexer->idx;
                kc_lex_reset_buffer(lexer);
                continue;
            case ')':
                push_token(&lexer->tokenlist, create_token(T_RPAREN, ")", false));
                ++lexer->idx;
                kc_lex_reset_buffer(lexer);
                continue;
            case '"':
                push_token(&lexer->tokenlist, create_token(T_QUOTE, "\"", false));
                ++lexer->idx;
                char* str = kc_lex_get_str(lexer, buffer);
                if (!(str)) {
                    lexer->error = true;
                    continue;
                }

                push_token(&lexer->tokenlist, create_token(T_STR, str, true));
                push_token(&lexer->tokenlist, create_token(T_QUOTE, "\"", false));
                ++lexer->idx;
                kc_lex_reset_buffer(lexer);
                continue;
        }

        kc_lex_bufpush(lexer, lexer->curChar);
        ++lexer->idx;
    }

    free(lexer->buffer);
    lexer->buffer = NULL;
}
