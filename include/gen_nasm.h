#ifndef GEN_NASM_H
#define GEN_NASM_H

#include "tree.h"
#include "table.h"

void gen_nasm(char* output, const Table* globals, const FunctionCollection* collection, const Node* tree);

#endif
