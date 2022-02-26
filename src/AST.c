#include "include/AST.h"



ast_node_t* kc_mkastnode(int op, ast_node_t* left, ast_node_t* right, int intval, ast_t* ast) {
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t*));
    node->op = op;
    node->left = left;
    node->right = right;
    node->intval = intval;
    ast->allocated[ast->size] = node;
    ++ast->size;

    return node;
}


ast_node_t* kc_mkastleaf(int op, int intval, ast_t* ast) {
    return kc_mkastnode(op, NULL, NULL, intval, ast);
}


ast_node_t* mkastunary(int op, ast_node_t* left, int intval, ast_t* ast) {
    return kc_mkastnode(op, left, NULL, intval, ast);
}


void ast_destroy(ast_t* ast) {
    for (int i = 0; i < ast->size; ++i) {
        free(ast->allocated[i]);
    }

    free(ast->allocated);
    ast->allocated = NULL;
}

