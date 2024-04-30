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
                 "\n_start:\n"
                 "\tcall main\n", 
                 globals_size);
    write_exit();
}

static void write_exit(void) {
    fprintf(out, "\tmov rdi, rax\n"
                 "\tmov rax, 60\n"
                 "\tsyscall\n");
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
            fprintf(out, "\n\t; negation unaire\n"
                         "\tpop rax\n"
                         "\tneg rax\n"
                         "\tpush rax\n");
        }
    } else {
        write_tree(globals, collection, fun, SECONDCHILD(tree));
        fprintf(out, "\n\t; opération binaire (%c)\n"
                     "\tpop rcx\n"
                     "\tpop rax\n"
                     "\t%s rax, rcx\n"
                     "\tpush rax\n",
                     tree->val.ident[0],
                     sym_op[(int)tree->val.ident[0]]);
    }

}
static void write_div_mod(const Table* globals, const FunctionCollection* collection,
                          const Function* fun, const Node* tree) {
    write_tree(globals, collection, fun, FIRSTCHILD(tree));
    write_tree(globals, collection, fun, SECONDCHILD(tree));
    if(tree->val.ident[0] == '/') {
        fprintf(out, "\n\t; division\n"
                     "\tmov rdx, 0\n"
                     "\tpop rcx\n"
                     "\tpop rax\n"
                     "\tidiv rcx\n"
                     "\tpush rax\n");
    } else if(tree->val.ident[0] == '%') {
        fprintf(out, "\n\t; modulo\n"
                     "\tpop rcx\n"
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
    if (fun->r_type != T_VOID) {
        write_tree(globals, collection, fun, FIRSTCHILD(tree));
        fprintf(out, "\n\t; chargement de la valeur de retour\n" 
                     "\tpop rax\n");
    }
    

    fprintf(out, "\n\t; retablissement de la pile avant retour\n"
                "\tmov rsp, rbp\n"
                 "\tpop rbp\n"
                 "\tret\n");
}

static void write_function(Function* fun) {
    fprintf(out, "\n; fonction %s\n"
                 "%s:\n"
                 "\t; sauvegarde du retour de pile\n"
                 "\tpush rbp\n"
                 "\tmov rbp, rsp\n"
                 "\n\t; réservation de mémoire pour les variables locales\n"
                 "\tsub rsp, %d\n\n"
                 "\t; corps de la fonction\n",
                 fun->name, fun->name, fun->locals.total_bytes);
}

static void write_assign(const Table* globals, const FunctionCollection* collection,
                         const Function* fun, const Node* tree) {
    
    //! Uncomment this when arrays evaluation is done
    // write_tree(globals, collection, fun, FIRSTCHILD(tree));
    write_tree(globals, collection, fun, SECONDCHILD(tree));
    
    Entry* entry;
    char* src;

    if ((entry = get_entry(&fun->locals, FIRSTCHILD(tree)->val.ident))) {
        src = "pop qword [rbp";
    } else if ((entry = get_entry(&fun->parameters, FIRSTCHILD(tree)->val.ident))) {
        // TODO: handle parameter access
    } else if ((entry = get_entry(globals, FIRSTCHILD(tree)->val.ident))) {
        src = "mov rcx, globals\n\tpop qword [rcx";
    }

    fprintf(out,
    "\n\t; assignation\n"
    "\t%s + %d]\n",
    src,
    entry->address);
}

static void write_load_ident(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree) {
    Entry* entry;
    if ((entry = get_entry(&fun->locals, tree->val.ident))) {
        fprintf(out, "\n\t; load local value store in %s\n"
                     "\tpush qword [rbp + %d]\n",
                     entry->name, entry->address);
    } else if ((entry = get_entry(&fun->parameters, tree->val.ident))) {
        // TODO: handle parameter access
    } else if ((entry = get_entry(globals, tree->val.ident))) {
        fprintf(out, "\n\t; load global value store in %s\n"
                     "\tmov rcx, globals\n"
                     "\tpush qword [rcx + %d]\n",
                     entry->name, entry->address);
    } else {
        // TODO: function call
    }
}

static void write_tree(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree) {
    if (!tree) return;
    
    //* return when its an expression
    //* break when its an instruction

    switch (tree->label) {
        case Assignation: write_assign(globals, collection, fun, tree); break;
        case Ident: write_load_ident(globals, collection, fun, tree); return;
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
        write_function(fun);
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

    fclose(out);
}
