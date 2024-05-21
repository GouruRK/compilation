#include <stdio.h>
#include <stdlib.h>

#include "sematic.h"
#include "tree.h"
#include "args.h"
#include "parser.h"
#include "table.h"
#include "errors.h"
#include "gen_nasm.h"

#define SYNTAX_ERROR   1
#define SEMANTIC_ERROR 2
#define OTHER_ERROR    3

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

/**
 * @brief Parse file and fill the abstract tree
 * 
 * @param args command line arguments
 * @param AST abstract tree to fill
 * @param print_tree indicate to print the tree
 * @return result of parser, 1 if error else 0
 */
int parse(Args args, Node** AST, bool print_tree) {
    int res;
    // choose input source 
    if (!args.source) {
        res = parse_file(stdin, AST);
    } else {
        res = parse_file(args.source, AST);
    }
    
    // print tree if no error
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
        // command line error
        return OTHER_ERROR;
    }
    // print help
    if (args.help) {
        print_help();
        return EXIT_SUCCESS;
    }

    // parsing input
    Node* AST = NULL;
    int res = parse(args, &AST, args.tree);

    if (res == 0) {
        // input was correctly parsed, proceed to the next step
        init_error(args.name ? args.name: "stdin");

        // initiate structures to check semantic and generate nasm
        int err_globals, err_functions;
        Table globals;
        FunctionCollection functions;
        err_globals = init_table(&globals);
        err_functions = init_function_collection(&functions);

        // error while initiating structures for semantic
        if (!err_functions || !err_globals) {
            goto exit_main;
        }
        // error while filling symbol tables
        if (!create_tables(&globals, &functions, AST)) {
            goto exit_main;
        }
        // print symbol tables
        if (args.symbols) {
            puts("globals:");
            print_table(globals);
            print_collection(functions);
        }
        // generating nasm if sematic is correct
        if (check_sem(&globals, &functions, AST)) {
            gen_nasm(args.name, &globals, &functions, AST);
        }
        // free allocated memory for semantic structures
        free_collection(&functions);
        free_table(&globals);
    }
    goto exit_main;
    exit_main:
        deleteTree(AST);
        print_rapport();
        return fatal_error() ? SEMANTIC_ERROR: res;
}
