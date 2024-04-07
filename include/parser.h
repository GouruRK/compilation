#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "tree.h"

int parse_file(FILE* source, Node** tree);

#endif
