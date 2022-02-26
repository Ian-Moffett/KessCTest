#include "include/Parser.h"

#ifdef KC_DUMP_TOKENS
static const char* const TOKENS_STR[] =  {
    "T_PRINT",
    "T_DIGIT",
    "T_LPAREN",
    "T_RPAREN",
    "T_STR",
    "T_INT",
    "T_QUOTE"
};
#endif


inline void parse(parser_t* parser) {
    ast_t ast = {
        .size = 0,
        .allocated = (ast_node_t**)malloc(sizeof(ast_node_t*)),
    };

    parser->ast = ast;

    while (parser->idx < parser->tokenlist.size) {
        parser->curToken = parser->tokenlist.tokens[parser->idx];
        #ifdef KC_DUMP_TOKENS
        printf("KC_TOKEN: %s => %s\n", TOKENS_STR[parser->curToken.type], parser->curToken.tok);
        #endif


        ++parser->idx;
    }
}
