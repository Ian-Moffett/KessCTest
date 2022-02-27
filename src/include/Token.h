#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>
#include <stdbool.h>

#define IS_DIGIT(ch) (ch >= '0' && ch <= '9')
#define IS_ALPHA_LOWER(ch) (ch >= 'a' && ch <= 'z')
#define IS_ALPHA_UPPER(ch) (ch >= 'A' && ch <= 'Z')
#define IS_WHITESPACE(ch) (ch == '\t' || ch == ' ')
#define IS_NEWLINE(ch) (ch == '\n')


typedef enum {
    T_PRINT, 
    T_DIGIT,
    T_LPAREN,
    T_RPAREN,
    T_STR,
    T_QUOTE,
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_EOL,
    T_SEMI,
    T_UINT8,
    T_UINT16,
    T_UINT32,
    T_UINT64,
    INVLD_TOKEN, 
} tokentype_t;


typedef struct {
    tokentype_t type;
    char* tok;
    bool alloc;
} token_t;


typedef struct {
    token_t* tokens;
    size_t size;
} tokenlist_t;


void destroy_tokenlist(tokenlist_t* tokenlist);
void push_token(tokenlist_t* tokenlist, token_t token);
token_t create_token(tokentype_t type, char* tok, bool alloc);


#endif
