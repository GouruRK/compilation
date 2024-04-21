#ifndef GEN_NASM_H
#define GEN_NASM_H

#include "tree.h"
#include "table.h"

void gen_nasm(char* output, Table* globals, FunctionCollection* collection, Node* tree);

#endif
