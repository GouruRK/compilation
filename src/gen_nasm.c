#include "gen_nasm.h"

#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 512

#define MIN(a, b) ((a) < (b) ? (a): (b))
#define MAX(a, b) ((a) > (b) ? (a): (b))

// nasm target
static FILE* out;

// builtin source file path 
static const char* buitlin_fcts[] = {
    "./builtin/getchar.asm",
    "./builtin/getint.asm",
    "./builtin/putchar.asm",
    "./builtin/putint.asm",
    NULL
};

// name of the builtin functions
static const char* buitlin_name[] = {
    "getchar",
    "getint",
    "putchar",
    "putint",
    NULL
};

// registers for arguments, according to AMD64 conventions
static const char* param_registers[] = {
    "rdi",
    "rsi",
    "rdx",
    "rcx",
    "r8",
    "r9",
    NULL
};

/**
 * @brief Remove path prefix to only keep the filename
 * 
 * @param path path prefix to substitute
 * @return modified path
 */
static char* sub_path(char* path);

/**
 * @brief Create the output file
 * 
 * @param output filename
 * @return 1 in case of success
 *         else 0
 */
static int create_file(char* output);

/**
 * @brief Copy `file` content to the output
 * 
 * @param file file to copy
 */
static void write_builtin(FILE* file);

/**
 * @brief Copy builtin functions in the output files
 * 
 * @return 1 in case of success
 *         else 0
 */
static int write_buitlins(void);

/**
 * @brief Write to output the nasm header
 *        If no buitlin functions are used, they'll not be copied in target
 * 
 * @param coll collection of function
 * @param globals_size size of globals variables in bytes
 */
static void write_init(const FunctionCollection* coll, int globals_size);

/**
 * @brief Write syscall to exit the programm
 * 
 */
static void write_exit(void);

/**
 * @brief Write nasm code instructions to handle nodes with 'AddSub' label
 *       and 'DivStar' label only if its a multiplication 
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function where the operation is computed
 * @param tree head node with the 'AddSub' or 'DivStar' label
 */
static void write_add_sub_mul(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, const Node* tree);

/**
 * @brief Write nasm code instructions to handle nodes with the 'DivStar' label
 *        in operations are division and modulo
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param fun current function where the operation is computed
 * @param tree head node with the 'DivStar' label
 */
static void write_div_mod(const Table* globals, const FunctionCollection* collection,
                          const Function* fun, const Node* tree);

/**
 * @brief Write nasm code for arithemtic operation where nodes labels are either
 *        'AddSub' or 'DivStar'
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param fun current function where the arithmetic is performed
 * @param tree head node with the 'AddSub' or 'DivStar' label
 */
static void write_arithmetic(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree);

/**
 * @brief Write nasm code to set rbp and rsp to their correct values when
 *        exiting a function
 * 
 */
static void write_function_exit(void);

/**
 * @brief Write nasm code to handle function returns
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param fun current function where the return is computed
 * @param tree head node with the 'Return' label
 */
static void write_return(const Table* globals, const FunctionCollection* collection,
                         const Function* fun, const Node* tree);

/**
 * @brief Write nasm code to handle function declaration, stack operations for
 *        parameters and memory allocation for locals
 * 
 * @param fun function to write declaration
 */
static void write_function(Function* fun);

/**
 * @brief Write assignation between an identifer and a value
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function where the assignation is computed
 * @param tree head node with the 'Assignation' label
 */
static void write_assign(const Table* globals, const FunctionCollection* collection,
                         const Function* fun, const Node* tree);

/**
 * @brief Write parameters assignation when calling a function. This function
 *        uses the AMD64 code conventions on parameters use
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function where the call is computed
 * @param tree first parameter node
 */
static void write_parameters(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree);

/**
 * @brief Write nasm code to handle function call
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param fun current function where the call is computed
 * @param tree head node with the 'Ident' label, the name of the function
 */
static void write_function_call(const Table* globals, const FunctionCollection* collection,
                                const Function* fun, const Node* tree);

/**
 * @brief Write nasm code to get variables values on top of the stack
 * 
 * @param globals global's table
 * @param collection collection of funtion
 * @param fun current function where the variable is used
 * @param tree head node with the 'Ident' label
 */
static void write_load_ident(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree);

/**
 * @brief Push on stack the number store in tree
 * 
 * @param tree 
 */
static void write_num(const Node* tree);

/**
 * @brief Push on stack the charactre stored in tree. In case of special
 *        caracters, push their numerical equivalent
 * 
 * @param tree 
 */
static void write_character(const Node* tree);

/**
 * @brief Write nasm code for each instruction of a function
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param fun function where the instructions are performed
 * @param tree node to write
 */
static void write_tree(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree);

/**
 * @brief Write all function declarations and their code
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param tree head of the programm, node with the 'Prog' label 
 */
static void write_functions(const Table* globals, const FunctionCollection* collection, const Node* tree);


static char* sub_path(char* path) {
    int len = strlen(path), i;
    for (i = len - 1; i > -1; i--) {
        if (path[i] == '/') {
            return path + i + 1;
        }
    }
    return path;
}

static int create_file(char* output) {
    if (!output) {
        output = "_anonymous";
    } else {
        output = sub_path(output);
        output[strlen(output) - 4] = '\0';
    }
    char filename[64];
    snprintf(filename, 64, "obj/%s.asm", output);

    out = fopen(filename, "w");
    return out != NULL;
}

static void write_builtin(FILE* file) {
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file)) {
        fputs(buffer, out);
    }
    fputc('\n', out);
}

static int write_buitlins(void) {
    FILE* bfile; 
    for (int i = 0; buitlin_fcts[i]; i++) {
        bfile = fopen(buitlin_fcts[i], "r");
        if (!bfile) {
            return 0;
        }
        write_builtin(bfile);
        fclose(bfile);
    }
    return 1;
}

static void write_init(const FunctionCollection* coll, int globals_size) {
    fprintf(out, "global _start\n"
                 "section .bss\n"
                 "\tglobals: resb %d\n"
                 "\nsection .text\n", globals_size);

    // check if the builtin functions are ever used to not surcharge the
    // generated nasm
    for (int i = 0; buitlin_name[i]; i++) {
        if (get_function(coll, buitlin_name[i])->is_used) {
            write_buitlins();
            break;
        }
    }

    fprintf(out, "\n_start:\n"
                 "\tcall main\n");
    write_exit();
}

static void write_exit(void) {
    fprintf(out, "\tmov rdi, rax\n"
                 "\tmov rax, 60\n"
                 "\tsyscall\n");
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

static void write_function_exit(void) {
    fprintf(out, "\n\t; retablissement de la pile avant retour\n"
                 "\tmov rsp, rbp\n"
                 "\tpop rbp\n"
                 "\tret\n");
}

static void write_return(const Table* globals, const FunctionCollection* collection,
                         const Function* fun, const Node* tree) {
    if (fun->r_type != T_VOID) {
        write_tree(globals, collection, fun, FIRSTCHILD(tree));
        fprintf(out, "\n\t; chargement de la valeur de retour\n" 
                     "\tpop rax\n");
    }
    write_function_exit();
}

static void write_function(Function* fun) {
    fprintf(out, "\n; fonction %s\n"
                 "%s:\n"
                 "\t; sauvegarde du retour de pile\n"
                 "\tpush rbp\n"
                 "\tmov rbp, rsp\n",
                 fun->name, fun->name);
    
    fprintf(out, "\n\t; On met les paramètres sur la pile\n");

    for (int i = MIN(fun->parameters.cur_len, 6) - 1; i > -1; i--) {
        fprintf(out, "\tpush %s\n", param_registers[i]);
    }

    fprintf(out, "\n\t; allocation de mémoire pour les variables locales\n"
                 "\tsub rsp, %d\n\n"
                 "\t; corps de la fonction\n",
                 fun->locals.total_bytes);

}

static void write_assign(const Table* globals, const FunctionCollection* collection,
                         const Function* fun, const Node* tree) {
    
    // ! Uncomment this when arrays evaluation is done
    // write_tree(globals, collection, fun, FIRSTCHILD(tree));
    write_tree(globals, collection, fun, SECONDCHILD(tree));
    
    Entry* entry;
    int index;
    if ((entry = get_entry(&fun->locals, FIRSTCHILD(tree)->val.ident))) {
        fprintf(out, "\n\t; assignation to '%s'\n"
                     "\tpop qword [rbp - %d]\n",
                     entry->name, fun->parameters.offset + entry->address);
    } else if ((entry = get_entry(&fun->parameters, FIRSTCHILD(tree)->val.ident))) {
        index = is_in_table(&fun->parameters, tree->val.ident);
        if (index < 6) {
            fprintf(out, "\n\t; assignation to '%s' after function call\n"
                         "\tpop qword [rbp - %d]\n", entry->name, entry->address);
        } else {
            fprintf(out, "\n\t; assignation to '%s' before function call\n"
                         "\tpop qword [rbp + %d]\n", entry->name, entry->address);
        }
    } else if ((entry = get_entry(globals, FIRSTCHILD(tree)->val.ident))) {
        fprintf(out, "\n\t; assignation to '%s'\n"
                     "\tmov rcx, globals\n"
                     "\tpop qword [rcx + %d]\n",
                     entry->name, entry->address);
    }

}

static void write_parameters(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree) {
    //* We want to treat the first parameter at the very last
    //* So parameters are handled in reverse order

    if (!tree) return;
    write_parameters(globals, collection, fun, tree->nextSibling);
    write_tree(globals, collection, fun, tree);
}

static void write_function_call(const Table* globals, const FunctionCollection* collection,
                                const Function* fun, const Node* tree) {
    Function* to_call = get_function(collection, tree->val.ident);
    
    if (FIRSTCHILD(tree)->label == ListExp) {
        write_parameters(globals, collection, fun, FIRSTCHILD(FIRSTCHILD(tree)));
        fprintf(out, "\n\t; on met les valeurs dans leurs registres\n");
        
        for (int i = 0; i < to_call->parameters.cur_len && i < 6; i++) {
            fprintf(out, "\tpop %s\n", param_registers[i]);
        }
    }
    fprintf(out, "\n\t; appel de fonction\n"
                 "\tcall %s\n",
                 tree->val.ident);
    
    if (to_call->r_type != T_VOID) {
        fprintf(out, "\n\t; on push la valeur de retour\n"
                     "\tpush rax\n");
    }
}

static void write_load_ident(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree) {
    Entry* entry;
    int index;
    if ((entry = get_entry(&fun->locals, tree->val.ident))) {
        fprintf(out, "\n\t; load local value store in %s\n"
                     "\tpush qword [rbp - %d]\n",
                     entry->name, fun->parameters.offset + entry->address);
    } else if ((entry = get_entry(&fun->parameters, tree->val.ident))) {
        index = is_in_table(&fun->parameters, tree->val.ident);
        if (index < 6) {
            fprintf(out, "\n\t; load parameter value store in '%s' after function call\n"
                         "\tpush qword [rbp - %d]\n", entry->name, entry->address);
        } else {
            fprintf(out, "\n\t; load parameter value store in '%s' before function call\n"
                         "\tpush qword [rbp + %d]\n", entry->name, entry->address);
        }
    } else if ((entry = get_entry(globals, tree->val.ident))) {
        fprintf(out, "\n\t; load global value store in %s\n"
                     "\tmov rcx, globals\n"
                     "\tpush qword [rcx + %d]\n",
                     entry->name, entry->address);
    } else {
        write_function_call(globals, collection, fun, tree);
    }
}

static void write_num(const Node* tree) {
    fprintf(out, "\n\t; lecture d'entier\n"
                 "\tpush %d\n",
                 tree->val.num);
}

static void write_character(const Node* tree) {
    int sym = -1;
    if (!strcmp(tree->val.ident, "'\\n'")) {
        sym = '\n';
    } else if (!strcmp(tree->val.ident, "'\\t'")) {
        sym = '\t';
    } else if (!strcmp(tree->val.ident, "'\\r'")) {
        sym = '\r';
    }

    if (sym == -1) {
        fprintf(out, "\n\t; lecture de charactere\n"
                     "\tpush %s\n",
                     tree->val.ident);
    } else {
        fprintf(out, "\n\t; lecture de charactere\n"
                     "\tpush %d\n",
                     sym);
    }

}

static void write_tree(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree) {
    if (!tree) return;
    
    switch (tree->label) {
        case Assignation: write_assign(globals, collection, fun, tree); return;
        case Ident: write_load_ident(globals, collection, fun, tree); return;
        case Num: write_num(tree); return;
        case Character: write_character(tree); return;
        case DivStar:
        case AddSub: write_arithmetic(globals, collection, fun, tree); return;
        case Return: write_return(globals, collection, fun, tree); return;
        default: return;
    }
}

static void write_functions(const Table* globals, const FunctionCollection* collection, const Node* tree) {
    Node* decl_fonct_node = FIRSTCHILD(SECONDCHILD(tree)), *head_instr;
    Function* fun;

    for (; decl_fonct_node != NULL;) {
        fun = get_function(collection,
                           SECONDCHILD(FIRSTCHILD(decl_fonct_node))->val.ident);
        
        head_instr = FIRSTCHILD(SECONDCHILD(SECONDCHILD(decl_fonct_node)));
        write_function(fun);


        for (; head_instr;) {
            write_tree(globals, collection, fun, head_instr);
            head_instr = head_instr->nextSibling;
        }
        write_function_exit();
        
        decl_fonct_node = decl_fonct_node->nextSibling;
    }
}

void gen_nasm(char* output, const Table* globals, const FunctionCollection* collection, const Node* tree) {
    if (!create_file(output)) {
        return;
    }
    write_init(collection, globals->total_bytes);

    write_functions(globals, collection, tree);

    fclose(out);
}
