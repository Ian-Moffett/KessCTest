#include "include/Parser.h"

#define KC_DUMP_TOKENS

#ifdef KC_DUMP_TOKENS
static const char* const TOKENS_STR[] =  {
    "T_PRINT",
    "T_DIGIT",
    "T_LPAREN",
    "T_RPAREN",
    "T_STR",
    "T_QUOTE",
    "T_PLUS",
    "T_MINUS",
    "T_STAR",
    "T_SLASH",
    "T_EOL",
    "T_SEMI",
};
#endif


static token_t peek(parser_t* parser, unsigned long long idx) {
    return parser->tokenlist.tokens[idx];
}


static void advance(parser_t* parser) {
    ++parser->idx;
    parser->curToken = parser->tokenlist.tokens[parser->idx];
}


bool isop(token_t token) {
    switch (token.type) {
        case T_PLUS:
        case T_MINUS:
        case T_STAR:
        case T_SLASH:
            return true;
    }

    return false;
}


inline void parse(parser_t* parser) {
    ast_t ast = {
        .size = 0,
        .nodes = (ast_node_t*)malloc(sizeof(ast_node_t)), 
    };

    parser->ast = ast;

    while (parser->idx < parser->tokenlist.size) {
        parser->curToken = parser->tokenlist.tokens[parser->idx];
        #ifdef KC_DUMP_TOKENS
        if (parser->curToken.type != T_DIGIT) {
            printf("KC_TOKEN: %s => %s\n", TOKENS_STR[parser->curToken.type], parser->curToken.tok);
        }
        #endif

        if (parser->curToken.type == T_DIGIT) {
        }

        ++parser->idx;
    }
}
