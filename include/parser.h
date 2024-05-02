#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "tree.h"

/**
 * @brief Function parsing a file
 * 
 * @param source 
 * @param tree 
 * @return int 
 */
int parse_file(FILE* source, Node** tree);

#endif
