#include <stdio.h>

#include "tree.h"
#include "args.h"
#include "parser.h"
#include "table.h"

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

    if (res == 0) {
        int err_globals, err_functions;
        Table globals;
        FunctionCollection functions;
        
        err_globals = init_table(&globals);
        err_functions = init_function_collection(&functions);

        if (err_functions && err_globals) {
            // create here tables and functions
        }
        free_collection(&functions);
        free_table(&globals);
    }
    deleteTree(AST);
    return res;
}
