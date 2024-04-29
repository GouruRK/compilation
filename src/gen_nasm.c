#include "gen_nasm.h"

#include <stdio.h>
#include <string.h>

static FILE* out;

static void write_init(int globals_size);
static void write_exit(void);
static int create_file(char* output);
static void write_tree(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree);
static void write_add_sub_mul(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, const Node* tree);
static void write_div_mod(const Table* globals, const FunctionCollection* collection,
                          const Function* fun, const Node* tree);
static void write_arithmetic(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree);
static void write_functions(const Table* globals, const FunctionCollection* collection, const Node* tree);
static void write_return(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree);

static void write_init(int globals_size) {
    fprintf(out, "extern putchar\n"
                 "extern getchar\n"
                 "extern putint\n"
                 "extern getint\n"
                 "section .bss\n"
                 "\tglobals: resb %d\n"
                 "\nsection .text\n"
                 "\nglobal _start\n\n"
                 "\n_start:\n", 
                 globals_size);
}

static void write_exit(void) {
    fprintf(out, "\npop rdi\n" //! change this when implementing function calls
                 "mov rax, 60\n"
                 "syscall\n\n");
}

static int create_file(char* output) {
    if (!output) {
        output = "_anonymous";
    } else {
        output[strlen(output) - 4] = '\0';
    }
    char filename[64];
    snprintf(filename, 64, "obj/%s.asm", output);

    out = fopen(filename, "w");
    return out != NULL;
}

static void write_add_sub_mul(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, const Node* tree) {
    static const char* sym_op[] = {
        ['-'] = "sub",
        ['+'] = "add",
        ['*'] = "imul"
    };

    write_tree(globals, collection, fun, FIRSTCHILD(tree));

    if (!SECONDCHILD(tree)) { // unary plus and minus
        if (tree->val.ident[0] == '-') {
            fprintf(out, "\tpop rcx\n"
                         "\tneg rcx\n"
                         "\tpush rcx\n");
        }
    } else {
        write_tree(globals, collection, fun, SECONDCHILD(tree));
        fprintf(out, "\tpop rcx\n"
                     "\tpop rax\n"
                     "\t%s rax, rcx\n"
                     "\tpush rax\n",
                     sym_op[(int)tree->val.ident[0]]);
    }

}
static void write_div_mod(const Table* globals, const FunctionCollection* collection,
                          const Function* fun, const Node* tree) {
    write_tree(globals, collection, fun, FIRSTCHILD(tree));
    write_tree(globals, collection, fun, SECONDCHILD(tree));
    if(tree->val.ident[0] == '/') {
        fprintf(out, "\tmov rdx, 0\n"
                 "\tpop rcx\n"
                 "\tpop rax\n"
                 "\tidiv rcx\n"
                 "\tpush rax\n");
    } else if(tree->val.ident[0] == '%') {
        fprintf(out, "\tpop rcx\n"
                 "\tpop rax\n"
                 "\tmov rdx, 0\n"
                 "\tidiv rcx\n"
                 "\tpush rdx\n");
    }
}

static void write_arithmetic(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree) {
    if (tree->val.ident[0] == '/' || tree->val.ident[0] == '%') {
        write_div_mod(globals, collection, fun, tree);
    } else {
        write_add_sub_mul(globals, collection, fun, tree);
    }
}

static void write_return(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree) {
    write_tree(globals, collection, fun, FIRSTCHILD(tree));
    // TODO: write 'ret'
}

static void write_tree(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree) {
    if (!tree) return;
    switch (tree->label) {
        case Num: fprintf(out, "\tpush %d\n", tree->val.num); return;
        case Character: fprintf(out, "\tpush '%c'\n", tree->val.c); return;
        case DivStar:
        case AddSub: write_arithmetic(globals, collection, fun, tree); return;
        case Return: write_return(globals, collection, fun, tree); return;
        default: break;
    }
    write_tree(globals, collection, fun, tree->nextSibling);
}

static void write_functions(const Table* globals, const FunctionCollection* collection, const Node* tree) {
    Node* decl_fonct_node = FIRSTCHILD(SECONDCHILD(tree));
    Function* fun;

    for (; decl_fonct_node != NULL;) {
        fun = get_function(collection,
            decl_fonct_node->firstChild->firstChild->nextSibling->val.ident);
        write_tree(globals, collection, fun, FIRSTCHILD(SECONDCHILD(SECONDCHILD(decl_fonct_node))));
        decl_fonct_node = decl_fonct_node->nextSibling;
    }
}

void gen_nasm(char* output, const Table* globals, const FunctionCollection* collection, const Node* tree) {
    if (!create_file(output)) {
        return;
    }
    write_init(globals->total_bytes);

    write_functions(globals, collection, tree);

    write_exit();
    fclose(out);
}
