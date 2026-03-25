#ifndef _AST_H_
#define _AST_H_

#include <stdio.h>

typedef struct {
    FILE * stream;
    int label_count;
} CodeGenContext;

typedef enum {
    NODE_IDENTIFIER,
    NODE_NUMBER,
    NODE_BLOCK,
    NODE_INSTRUCTION,
    NODE_READ,
    NODE_PRINT,
    NODE_AFFECTATION,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    // statements
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    // boolean
    NODE_HASH,
    NODE_EQUALS,
    NODE_OR,
    NODE_AND,
    NODE_LESSER_THAN,
    NODE_GREATER_THAN,
    NODE_LESSER_EQUALS,
    NODE_GREATER_EQUALS,
} ASTNodeType;

#endif // _AST_H_
