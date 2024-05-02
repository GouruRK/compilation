#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "table.h"

/**
 * @brief Sacred function of semantic things
 * 
 * @param globals symbol table
 * @param collection array of function symbol
 * @param tree pointer of the start of tree
 * @return int 
 */
int check_sem(Table* globals, FunctionCollection* collection, Node* tree);

#endif
