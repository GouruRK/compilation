#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>
#include <stdio.h>

typedef struct {
    bool help;
    bool tree;
    bool err;
    FILE* source;
} Args;

/**
 * @brief Fonction to do initialisation of arguments
 * 
 * @return Args 
 */
Args init_args(void);

/**
 * @brief Fonction to use arguments passed in command line
 * 
 * @param argc 
 * @param argv 
 * @return Args 
 */
Args parse_args(int argc, char* argv[]);

#endif
