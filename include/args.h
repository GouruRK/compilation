#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>
#include <stdio.h>

typedef struct {
    bool help;
    bool tree;
    bool err;
    bool symbols;
    char* name;
    char* ouput;
    FILE* source;
} Args;

/**
 * @brief Fonction to use arguments passed in command line
 * 
 * @param argc 
 * @param argv 
 * @return Args 
 */
Args parse_args(int argc, char* argv[]);

#endif
