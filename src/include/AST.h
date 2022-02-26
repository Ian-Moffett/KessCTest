#ifndef AST_H
#define AST_H

#include <stdlib.h>

typedef enum {
    A_ADD,
    A_SUB,
    A_MUL,
    A_DIV,
    A_INT
} ast_node_type_t;


typedef struct AST_NODE {
    int op;
    struct AST_NODE* left;
    struct AST_NODE* right;
    int intval;
} ast_node_t;

typedef struct {
    unsigned long long size;
    ast_node_t** allocated;
} ast_t;



ast_node_t* kc_mkastnode(int op, ast_node_t* left, ast_node_t* right, int intvalue, ast_t* ast);
ast_node_t* kc_mkastleaf(int op, int intval, ast_t* ast);
ast_node_t* mkastunary(int op, ast_node_t* left, int intval, ast_t* ast);
void ast_destroy(ast_t* ast);

#endif
