#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "ast.h"
#include <stdio.h>

typedef struct {
  FILE *stream;
  FacileSymbol *table;
  int label_count;
  int current_loop_label;
} CodeGenContext;

#endif // _CONTEXT_H_
