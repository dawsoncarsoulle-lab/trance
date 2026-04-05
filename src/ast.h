#ifndef AST_H_
#define AST_H_

#include <stdlib.h>

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
