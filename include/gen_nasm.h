#ifndef GEN_NASM_H
#define GEN_NASM_H

#include "tree.h"
#include "table.h"

/**
 * @brief Function to generate nasm file
 * 
 * @param output name of file
 * @param globals symbols table
 * @param collection array of functions
 * @param tree pointer to tree
 */
void gen_nasm(char* output, const Table* globals, const FunctionCollection* collection, const Node* tree);

#endif
