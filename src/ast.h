#ifndef AST_H_
#define AST_H_

#include <stdlib.h>

typedef enum {
  NODE_BLOCK,
  // statements/instructions
  NODE_READ,
  NODE_PRINT,
  NODE_AFFECTATION,
  NODE_IF_STATEMENT,
  NODE_WHILE_STATEMENT,
  NODE_BREAK,
  NODE_CONTINUE,
  // expression
  NODE_NUMBER,
  NODE_IDENTIFIER,
  NODE_ADD,
  NODE_SUB,
  NODE_MUL,
  NODE_DIV,
  // boolean
  NODE_TRUE,
  NODE_FALSE,
  NODE_NOT,
  NODE_HASH,
  NODE_EQUALS,
  NODE_OR,
  NODE_AND,
  NODE_LESSER_THAN,
  NODE_GREATER_THAN,
  NODE_LESSER_EQUALS,
  NODE_GREATER_EQUALS,
} ASTNodeType;

typedef struct {
  char *key;
  int value;
} FacileSymbol;

#define NODE_MAX_CHILDREN 3

typedef struct FacileNode {
  int type;
  int data; // integer data or FacileSymbol value (identifier)
  struct FacileNode *children[NODE_MAX_CHILDREN];
} FacileNode;

static inline FacileNode *facile_create_node(int type) {
  FacileNode *node = (FacileNode *)malloc(sizeof(*node));
  node->type = type;
  node->data = 0;
  node->children[0] = NULL;
  node->children[1] = NULL;
  node->children[2] = NULL;
  return node;
}

static inline void facile_node_add_child(FacileNode *parent, FacileNode *child) {
  for (int i = 0; i < NODE_MAX_CHILDREN; i++)
    if (parent->children[i] == NULL) {
      parent->children[i] = child;
      return;
    }
}

static inline void facile_node_free(FacileNode *node) {
  if (!node)
    return;
  for (int i = 0; i < NODE_MAX_CHILDREN; i++)
    facile_node_free(node->children[i]);
  free(node);
}

#endif // AST_H_
