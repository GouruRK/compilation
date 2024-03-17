#include <stdio.h>

#include "tree.h"
#include "args.h"
#include "parser.h"

/**
 * @brief Display help message
 */
void print_help(void) {
    printf("Usage: ./tpcas [OPTION...] FILE\n"
           "Check if syntax of given file is valid, according to the grammar defined in parser.y\n\n"
           "With no FILE, FILE is the standard input\n\n"
           "  -t, --tree\t\tprint abstract tree of the given file\n"
           "  -h, --help\t\tdisplay this help message and exit\n"
           );
}

int parse(FILE* source, Node** AST, bool print_tree) {
    int res = parse_file(source ? source: stdin, AST);
    if (!res && print_tree) {
        printTree(*AST);
    }
    if (source) {
        fclose(source);
    }
    return res;
}

int main(int argc, char* argv[]) {
    Args args = parse_args(argc, argv);
    if (args.err) {
        return 2;
    }
    if (args.help) {
        print_help();
        return 0;
    }

    Node* AST = NULL;

    int res = parse(args.source, &AST, args.tree);
    
    if (AST) {
        deleteTree(AST);
    }
    return res;
}
