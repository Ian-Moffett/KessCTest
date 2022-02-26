#include "include/Parser.h"



inline void parse(parser_t* parser) {
    ast_t kc_ast = {
        .size = 0,
        .nodes = (ast_node_t*)malloc(sizeof(ast_node_t)),
    };

    parser->ast = kc_ast;
}
