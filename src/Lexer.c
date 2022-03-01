#include "include/Lexer.h"


static void kc_lex_reset_buffer(lexer_t* lexer) {
    memset(lexer->buffer, '\0', strlen(lexer->buffer));
    lexer->bufferidx = 0;
    lexer->buffer = (char*)realloc(lexer->buffer, sizeof(char));
}


static void kc_lex_bufpush(lexer_t* lexer, char c) {
    lexer->buffer[lexer->bufferidx] = c;
    ++lexer->bufferidx;
}


static char* kc_lex_getident(lexer_t* lexer, char* buffer) {
    char* identbuf = (char*)calloc(2, sizeof(char));
    unsigned int idbidx = 0;

    while (lexer->idx < strlen(buffer) && buffer[lexer->idx] == ' ') {++lexer->idx;}

    lexer->curChar = buffer[lexer->idx];

    if (IS_DIGIT(lexer->curChar)) {
        free(identbuf);
        return NULL;
    }

    while (lexer->idx < strlen(buffer)) {
        lexer->curChar = buffer[lexer->idx];

        if (!(IS_DIGIT(lexer->curChar)) && !(IS_ALPHA_LOWER(lexer->curChar)) && !(IS_ALPHA_UPPER(lexer->curChar)) && lexer->curChar != '_') {
            break;
        }

        identbuf[idbidx] = lexer->curChar;
        ++idbidx;
        ++lexer->idx;
        identbuf = (char*)realloc(identbuf, sizeof(char) * (idbidx + 2));
    }

    if (strlen(identbuf) == 0) {
        free(identbuf);
        return NULL;
    }
        
    return identbuf;
}


static tokentype_t kc_lex_buffervalid(lexer_t* lexer, char* buffer) {
    if (strcmp(lexer->buffer, "printf") == 0) {
        push_token(&lexer->tokenlist, create_token(T_PRINT, "printf", false));
        return T_PRINT;
    } else if (strcmp(lexer->buffer, "uint8") == 0) {
        char* buffercpy = (char*)calloc(strlen(lexer->buffer) + 2, sizeof(char));
        strcpy(buffercpy, lexer->buffer);
        push_token(&lexer->tokenlist, create_token(T_UINT8, buffercpy, true));

        char* ident = kc_lex_getident(lexer, buffer);

        if (ident) {             
            push_token(&lexer->tokenlist, create_token(T_IDENTIFIER, ident, true));
        }

        return T_UINT8;
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
            kc_log_err("SyntaxError: Expected '\"' before EOL.", "", lexer->line);
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

static char* kc_lex_get_int(lexer_t* lexer, char* buffer) {
    char* decbuf = (char*)calloc(2, sizeof(char));
    unsigned int dbidx = 0;

    while (lexer->idx < strlen(buffer)) {
        lexer->curChar = buffer[lexer->idx];

        if (!(IS_DIGIT(lexer->curChar))) {
            break;
        }

        decbuf[dbidx] = lexer->curChar;
        ++dbidx;
        decbuf = (char*)realloc(decbuf, sizeof(char) * (dbidx + 2));
        ++lexer->idx;
    }

    return decbuf;
}


static bool kc_lex_nondigit(char* buffer) {
    for (int i = 0; i < strlen(buffer); ++i) {
        if (!(IS_DIGIT(buffer[i])) && buffer[i] != ' ') {
            return true;
        }
    }

    return false;
}


tokenlist_t kc_lex_tokenize(lexer_t* lexer, char* buffer) {
    while (lexer->idx < strlen(buffer) && !(lexer->error)) {
        lexer->curChar = buffer[lexer->idx];   
 
        if (IS_DIGIT(lexer->curChar)) {  
            /*
            if (!(kc_lex_iswhitespace(lexer->buffer)) && strlen(lexer->buffer) > 0 && kc_lex_buffervalid(lexer, buffer) == INVLD_TOKEN) {
                kc_log_err("SyntaxError: Unexpected token found while lexing.", lexer->buffer, lexer->line);
                lexer->error = true;
                continue;
            }
            */

            char* digit = kc_lex_get_int(lexer, buffer);

            for (int i = 0; i < strlen(digit); ++i) {
                kc_lex_bufpush(lexer, digit[i]);
            }

            tokentype_t type = kc_lex_buffervalid(lexer, buffer);

            switch (type) {
                case T_UINT8:
                    free(digit);
                    break;
                default:
                    push_token(&lexer->tokenlist, create_token(T_DIGIT, digit, true)); 
                    continue;
            }

            kc_lex_reset_buffer(lexer);
            continue;
        }

        if (IS_NEWLINE(lexer->curChar) || IS_WHITESPACE(lexer->curChar)) {
            if (!(kc_lex_iswhitespace(lexer->buffer) && strlen(lexer->buffer) > 0 && kc_lex_buffervalid(lexer, buffer) == INVLD_TOKEN)) {
                if (kc_lex_nondigit(lexer->buffer)) {
                    kc_log_err("SyntaxError: Unexpected token found while lexing.", lexer->buffer, lexer->line);
                    lexer->error = true;
                }
            }
            
            ++lexer->idx;
            continue;
        }
            
        switch (lexer->curChar) {
            case '=':
                push_token(&lexer->tokenlist, create_token(T_EQUALS, "=", false));
                kc_lex_reset_buffer(lexer);
                ++lexer->idx;
                continue;
            case ';':
                push_token(&lexer->tokenlist, create_token(T_SEMI, ";", false));
                kc_lex_reset_buffer(lexer);
                ++lexer->idx;
                continue;
            case '(':
                ++lexer->idx;
                char* id = kc_lex_getident(lexer, buffer); 

                if (!(kc_lex_iswhitespace(lexer->buffer)) && strlen(lexer->buffer) > 0 && kc_lex_buffervalid(lexer, buffer) == INVLD_TOKEN) {
                    kc_log_err("SyntaxError: Unexpected token found while lexing.", lexer->buffer, lexer->line);
                    lexer->error = true;
                    continue;
                }

                push_token(&lexer->tokenlist, create_token(T_LPAREN, "(", false));
                kc_lex_reset_buffer(lexer);

                if (id) {
                    push_token(&lexer->tokenlist, create_token(T_IDENTIFIER, id, true));
                }

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
            case '+':
                push_token(&lexer->tokenlist, create_token(T_PLUS, "+", false));
                ++lexer->idx;
                kc_lex_reset_buffer(lexer);
                continue;
            case '*':
                push_token(&lexer->tokenlist, create_token(T_STAR, "*", false));
                ++lexer->idx;
                kc_lex_reset_buffer(lexer);
                continue;
            case '-':
                push_token(&lexer->tokenlist, create_token(T_MINUS, "-", false));
                ++lexer->idx;
                kc_lex_reset_buffer(lexer);
                continue;
            case '/':
                push_token(&lexer->tokenlist, create_token(T_SLASH, "/", false));
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
