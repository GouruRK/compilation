#include <stdio.h>

#include "sematic.h"
#include "tree.h"
#include "args.h"
#include "parser.h"
#include "table.h"
#include "errors.h"
#include "gen_nasm.h"

#define SEMANTIC_ERROR 2

/**
 * @brief Display help message
 */
void print_help(void) {
    printf("Usage: ./tpcc [OPTION...] FILE\n"
           "Check if syntax of given file is valid, according to the grammar defined in parser.y\n\n"
           "With no FILE, FILE is the standard input\n\n"
           "  -t, --tree\t\tprint abstract tree of the given file\n"
           "  -s, --symbols\t\tprint associated symbol tables\n"
           "  -h, --help\t\tdisplay this help message and exit\n"
           );
}

int parse(Args args, Node** AST, bool print_tree) {
    int res;
    if (!args.source) {
        res = parse_file(stdin, AST);
    } else {
        res = parse_file(args.source, AST);
    }
    
    if (!res && print_tree) {
        printTree(*AST);
    }
    if (args.source) {
        fclose(args.source);
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

    int res = parse(args, &AST, args.tree);

    if (res == 0) {
        init_error(args.name ? args.name: "stdin");
        int err_globals, err_functions;
        Table globals;
        FunctionCollection functions;
        
        err_globals = init_table(&globals);
        err_functions = init_function_collection(&functions);

        if (err_functions && err_globals) {
            if (create_tables(&globals, &functions, AST) && args.symbols) {
                puts("globals:");
                print_table(globals);
                print_collection(functions);
            }
            
        }
        if (!fatal_error()) {
            if (check_sem(&globals, &functions, AST)) {
                //! TODO: uncomment this when generating nasm
                gen_nasm(args.name, &globals, &functions, AST);
            }
        }

        free_collection(&functions);
        free_table(&globals);
    }
    deleteTree(AST);
    return fatal_error() ? SEMANTIC_ERROR: res;
}
