#include "gen_nasm.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct  {
    char* symbol;
    char* instr;
} comp_op;

#define BUFFER_SIZE 512
#define DEFAULT_PATH ""

// nasm target
static FILE* out;

// builtin source file path 
static const char* buitlin_fcts[] = {
    "./builtin/getchar.asm", "./builtin/getint.asm",
    "./builtin/putchar.asm", "./builtin/putint.asm",
    NULL
};

// registers for arguments, according to AMD64 conventions
static const char* param_registers[] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9", NULL
};

static const comp_op operators[] = {
    {.symbol = "==", .instr = "je"},
    {.symbol = "!=", .instr = "jne"},
    {.symbol = "<",  .instr = "jl"},
    {.symbol = "<=", .instr = "jle"},
    {.symbol = ">",  .instr = "jg"},
    {.symbol = ">=", .instr = "jge"},
    {NULL,           NULL}
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
 * @brief Write nasm code to access to local variables
 * 
 * @param fun function where the user access the local
 * @param entry entry the user is accessing
 * @param instr instruction to compute (between 'push' and 'pop')
 * @param address if we tried to access a local address (like arrays given to a
 *                function call)
 */
static void local_access(const Function* fun, const Entry* entry, 
                         const char* instr, bool address);

/**
 * @brief Write nasm code to access to parameters
 * 
 * @param fun function where the user access the parameter
 * @param entry entry the user is accessing
 * @param instr instruction to compute (between 'push' and 'pop')
 * @param address if we tried to access a parameter address (like arrays given
 *                to a function call)
 */
static void param_access(const Function* fun, const Entry* entry,
                         const char* instr, bool address);

/**
 * @brief Write nasm code to access to global variables
 * 
 * @param entry entry the user is accessing
 * @param instr instruction to compute (between 'push' and 'pop')
 * @param address if we tried to access a global address (like arrays given to a
 *                function call)
 */
static void global_access(const Entry* entry, const char* instr, bool address);

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
 * @brief Get the nasm instruction for the given comparaison symbol
 * 
 * @param symbol 
 * @return
 */
static char* get_comp_instr(const char* symbol);

/**
 * @brief Give a number of an unsued label
 * 
 * @return
 */
static int next_free_label(void);

/**
 * @brief Write nasm code to handle comparaisons
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param fun function where the comparaison is computed
 * @param tree head node with either the 'Order' or the 'Eq' label
 */
static void write_comp(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree);

/**
 * @brief Write the boolean transformation from a non-null variable to '1'
 *        or keep 0 if not
 * 
 */
static void write_bool_transform(void);

/**
 * @brief Write nasm code to handle 'and' (&&) lazy evaluation
 * 
 * @param globals global's table
 * @param collection collection of fucntion
 * @param fun function where the 'and' is computed
 * @param tree head node with the 'And' label
 */
static void write_and(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree);

/**
 * @brief Write nasm code to handle 'or' (||) lazy evaluation
 * 
 * @param globals global's table
 * @param collection collection of fucntion
 * @param fun function where the 'or' is computed
 * @param tree head node with the 'Or' label
 */
static void write_or(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree);

/**
 * @brief Write nasm code to handle negation
 * 
 * @param globals global's table
 * @param collection collection of fucntion
 * @param fun function where the 'negation' is computed
 * @param tree head node with the 'Negation' label
 */
static void write_neg(const Table* globals, const FunctionCollection* collection,
                      const Function* fun, const Node* tree);

/**
 * @brief Write nasm code to handle 'if' and 'else' statements
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param fun function where the 'if' is computed
 * @param tree head node with the 'If' label
 */
static void write_if(const Table* globals, const FunctionCollection* collection,
                     const Function* fun, const Node* tree);

/**
 * @brief Write nasm code to handle the 'while' statement
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param fun function where the 'while' is computed
 * @param tree head node with the 'While' label
 */
static void write_while(const Table* globals, const FunctionCollection* collection,
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
 * @brief Write bloc of instruction
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param fun function where the instructions are performed
 * @param tree node to write
 */
static void write_instructions(const Table* globals, const FunctionCollection* collection,
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
    snprintf(filename, 64, DEFAULT_PATH"%s.asm", output);

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

    write_buitlins();

    fprintf(out, "\n_start:\n"
                 "\tcall\tmain\n");
    write_exit();
}

static void write_exit(void) {
    fprintf(out, "\tmov \trdi, rax\n"
                 "\tmov \trax, 60\n"
                 "\tsyscall\n");
}


static void write_add_sub_mul(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, const Node* tree) {
    static const char* sym_op[] = {
        ['-'] = "sub ",
        ['+'] = "add ",
        ['*'] = "imul"
    };

    write_tree(globals, collection, fun, FIRSTCHILD(tree));

    if (!SECONDCHILD(tree)) { // unary plus and minus
        if (tree->val.ident[0] == '-') {
            fprintf(out, "\n\t; negation unaire\n"
                         "\tpop \trax\n"
                         "\tneg \trax\n"
                         "\tpush\trax\n");
        }
    } else {
        write_tree(globals, collection, fun, SECONDCHILD(tree));
        fprintf(out, "\n\t; opération binaire (%c)\n"
                     "\tpop \trcx\n"
                     "\tpop \trax\n"
                     "\t%s\trax, rcx\n"
                     "\tpush\trax\n",
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
                     "\tpop \trcx\t; diviseur\n"
                     "\tpop \trax\n"
                     "\tcqo \t; initialise le reste\n"
                     "\tidiv\trcx\n"
                     "\tpush\trax\n");
    } else if(tree->val.ident[0] == '%') {
        fprintf(out, "\n\t; modulo\n"
                     "\tpop \trcx\t; diviseur\n"
                     "\tpop \trax\n"
                     "\tcqo \t; initialise le reste\n"
                     "\tidiv\trcx\n"
                     "\tpush\trdx\n");
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
                 "\tmov \trsp, rbp\n"
                 "\tpop \trbp\n"
                 "\tret\n");
}

static void write_return(const Table* globals, const FunctionCollection* collection,
                         const Function* fun, const Node* tree) {
    if (fun->r_type != T_VOID) {
        write_tree(globals, collection, fun, FIRSTCHILD(tree));
        fprintf(out, "\n\t; chargement de la valeur de retour\n" 
                     "\tpop \trax\n");
    }
    write_function_exit();
}

static void write_function(Function* fun) {
    fprintf(out, "\n; fonction %s\n"
                 "%s:\n"
                 "\t; sauvegarde du retour de pile\n"
                 "\tpush\trbp\n"
                 "\tmov \trbp, rsp\n",
                 fun->name, fun->name);
    
    fprintf(out, "\n\t; On met les paramètres sur la pile\n");

    for (int i = 0; i < fun->parameters.cur_len && param_registers[i]; i++) {
        fprintf(out, "\tpush\t%s\n", param_registers[i]);
    }

    fprintf(out, "\n\t; allocation de mémoire pour les variables locales\n"
                 "\tsub \trsp, %d\n\n"
                 "\t; corps de la fonction\n",
                 fun->locals.total_bytes);

}

static void write_assign(const Table* globals, const FunctionCollection* collection,
                         const Function* fun, const Node* tree) {
    
    write_tree(globals, collection, fun, SECONDCHILD(tree));
    
    Entry* entry;
    if ((entry = get_entry(&fun->locals, FIRSTCHILD(tree)->val.ident))) {
        if (is_array(entry->type)) {
            write_tree(globals, collection, fun, FIRSTCHILD(FIRSTCHILD(tree)));
        }
        local_access(fun, entry, "pop ", false);
    } else if ((entry = get_entry(&fun->parameters, FIRSTCHILD(tree)->val.ident))) {
        if (is_array(entry->type)) {
            write_tree(globals, collection, fun, FIRSTCHILD(FIRSTCHILD(tree)));
        }
        param_access(fun, entry, "pop ", false);
    } else if ((entry = get_entry(globals, FIRSTCHILD(tree)->val.ident))) {
        if (is_array(entry->type)) {
            write_tree(globals, collection, fun, FIRSTCHILD(FIRSTCHILD(tree)));
        }
        global_access(entry, "pop ", false);
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
            fprintf(out, "\tpop \t%s\n", param_registers[i]);
        }
    }
    fprintf(out, "\n\t; appel de fonction\n"
                 "\tcall\t%s\n",
                 tree->val.ident);

    if (to_call->parameters.cur_len > 6) {
        fprintf(out, "\n\t; on enleve les parametres qui sont restes dans la pile\n"
                     "\tadd \trsp, %d\n",
                     (to_call->parameters.cur_len - 6)*8);
    }

    if (to_call->r_type != T_VOID) {
        fprintf(out, "\n\t; on push la valeur de retour\n"
                     "\tpush\trax\n");
    }
}

static void local_access(const Function* fun, const Entry* entry, 
                         const char* instr, bool address) {
    if (is_array(entry->type)) {
        if (address) {
            fprintf(out, "\n\t; accessing to '%s' in locals\n"
                        "\tmov \trax, rbp\n"
                        "\tsub \trax, %d\n"
                        "\t%s\trax\n",
                        entry->name, fun->parameters.offset + entry->address, instr);
            return;
        }
        fprintf(out, "\n\t; accessing to '%s' in locals\n"
                     "\tpop \trcx\n"
                     "\timul\trcx, 8\n"
                     "\tmov \trax, rbp\n"
                     "\tsub \trax, %d\n"
                     "\tsub \trax, rcx\n"
                     "\t%s\tqword [rax]\n",
                     entry->name, fun->parameters.offset + entry->address, instr);
    } else {
        fprintf(out, "\n\t; accessing to '%s' in locals\n"
                     "\t%s\tqword [rbp - %d]\n",
                     entry->name, instr, fun->parameters.offset + entry->address);

    }
}

static void param_access(const Function* fun, const Entry* entry,
                         const char* instr, bool address) {
    int index = is_in_table(&fun->parameters, entry->name);
    if (index < 6) {
        if (is_array(entry->type)) {
            if (address) {
                fprintf(out, "\n\t; accessing address of '%s' in parameters\n"
                             "\tmov \trax, rbp\n"
                             "\tsub \trax, %d\n"
                             "\t%s\tqword [rax]\n",
                             entry->name, entry->address, instr);
                return;
            }
            fprintf(out, "\n\t; accessing to '%s' in parameters\n"
                         "\tpop \trcx\n"
                         "\timul\trcx, 8\n"
                         "\tmov \trax, rbp\n"
                         "\tsub \trax, %d\n"
                         "\tmov \trdx, qword [rax]\n"
                         "\tsub \trdx, rcx\n"
                         "\t%s\tqword [rdx]\n",
                         entry->name, entry->address, instr);
        } else {
            fprintf(out, "\n\t; accessing to '%s' in parameters\n"
                         "\t%s\tqword [rbp - %d]\n",
                         entry->name, instr, entry->address);
        }
    } else {
        if (is_array(entry->type)) {
            if (address) {
                fprintf(out, "\n\t; accessing address of '%s' in parameters\n"
                             "\tmov \trax, rbp\n"
                             "\tadd \trax, %d\n"
                             "\t%s\tqword [rax]\n",
                             entry->name, entry->address, instr);
                return;
            }
            fprintf(out, "\n\t; accessing to '%s' in parameters\n"
                         "\tpop \trcx\n"
                         "\timul\trcx, 8\n"
                         "\tmov \trax, rbp\n"
                         "\tsub \trax, %d\n"
                         "\tadd \trax, rcx\n"
                         "\t%s\tqword [rax]\n",
                         entry->name, entry->address, instr);
        } else {
            fprintf(out, "\n\t; accessing to '%s' in parameters\n"
                        "\t%s\tqword [rbp + %d]\n",
                        entry->name, instr, entry->address);
        }
    }
}

static void global_access(const Entry* entry, const char* instr, bool address) {
    if (is_array(entry->type)) {
        if (address) {
            fprintf(out, "\n\t; accessing  address of '%s' in globals\n"
                         "\tmov \trcx, globals\n"
                         "\tadd \trcx, %d\n"
                         "\t%s\trcx\n",
                         entry->name, entry->address, instr);
            return;
        }
        fprintf(out, "\n\t; accessing to '%s' in globals\n"
                     "\tpop \trcx\n"
                     "\timul\trcx, 8\n"
                     "\tmov \trax, globals\n"
                     "\tadd \trax, %d\n"
                     "\tadd \trax, rcx\n"
                     "\t%s\tqword [rax]\n",
                     entry->name, entry->address, instr);
    } else {
        fprintf(out, "\n\t; accessing to '%s' in globals\n"
                     "\tmov \trcx, globals\n"
                     "\t%s\tqword [rcx + %d]\n",
                     entry->name, instr, entry->address);
    }
}

static void write_load_ident(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Node* tree) {
    write_tree(globals, collection, fun, FIRSTCHILD(tree));
    Entry* entry;
    if ((entry = get_entry(&fun->locals, tree->val.ident))) {
        local_access(fun, entry, "push", !FIRSTCHILD(tree));
    } else if ((entry = get_entry(&fun->parameters, tree->val.ident))) {
        param_access(fun, entry, "push", !FIRSTCHILD(tree));
    } else if ((entry = get_entry(globals, tree->val.ident))) {
        global_access(entry, "push", !FIRSTCHILD(tree));
    } else {
        write_function_call(globals, collection, fun, tree);
    }
}

static char* get_comp_instr(const char* symbol) {
    for (int i = 0; operators[i].symbol; i++) {
        if (!strcmp(operators[i].symbol, symbol)) {
            return operators[i].instr;
        }
    }
    return NULL;
}

static int next_free_label(void) {
    static int label = 0;
    return label++;
}

static void write_comp(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree) {
    write_tree(globals, collection, fun, FIRSTCHILD(tree));
    write_tree(globals, collection, fun, SECONDCHILD(tree));

    fprintf(out, "\n\t; chargement des valeurs pour la comparaison\n"
                 "\tpop \trcx\n"
                 "\tpop \trax\n");
    
    int nlabel = next_free_label();
    int ncontinue = next_free_label();

    fprintf(out, "\n\t; comparaison (%s)\n"
                 "\tcmp \trax, rcx\n"
                 "\t%s \tlabel%d\n"
                 "\tpush\t0\n"
                 "\tjmp \tcontinue%d\n"
                 "\tlabel%d:\n"
                 "\tpush\t1\n"
                 "\tcontinue%d:\n",
                 tree->val.ident, get_comp_instr(tree->val.ident), nlabel,
                 ncontinue, nlabel, ncontinue);
}

static void write_bool_transform(void) {
    int nlabel = next_free_label();
    int ncontinue = next_free_label();

    fprintf(out, "\n\t; transform output to correct format\n"
                 "\tpop \trax\n"
                 "\tcmp \trax, 0\n"
                 "\tjne \t label%d\n"
                 "\tpush\t0\n"
                 "\tjmp \tcontinue%d\n"
                 "\tlabel%d:\n"
                 "\tpush\t1\n"
                 "\tcontinue%d:\n",
                 nlabel, ncontinue, nlabel, ncontinue);
}

static void write_and(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree) {
    int nlabel = next_free_label();
    int ncontinue = next_free_label();

    fprintf(out, "\n\t; evaluation d'un 'et' (&&)\n"
                 "\n\t; evaluation du membre gauche\n");

    write_tree(globals, collection, fun, FIRSTCHILD(tree));

    fprintf(out, "\n\t; evaluation du 'et' (&&)\n"
                 "\tpop \trax\n"
                 "\tcmp \trax, 0\n"
                 "\tjne \tlabel%d\n"
                 "\tpush\t0\n"
                 "\tjmp \tcontinue%d\n"
                 "\tpush\t1\n"
                 "\tlabel%d:\n",
                 nlabel, ncontinue, nlabel);

    write_tree(globals, collection, fun, SECONDCHILD(tree));

    fprintf(out, "\tcontinue%d:\n", ncontinue);
    write_bool_transform();
}

static void write_or(const Table* globals, const FunctionCollection* collection,
                     const Function* fun, const Node* tree) {
    int nlabel = next_free_label();
    int ncontinue = next_free_label();

    fprintf(out, "\n\t; evaluation d'un 'ou' (||)\n"
                 "\n\t; evaluation du membre gauche\n");

    // write condition
    write_tree(globals, collection, fun, FIRSTCHILD(tree));

    // transform non-boolean values to boolean
    write_bool_transform();

    // evaluate condition
    fprintf(out, "\n\t; evaluation du 'or' (||)\n"
                 "\tpop \trax\n"
                 "\tcmp \trax, 1\n"
                 "\tjne \tlabel%d\n"
                 "\tpush\t1\n"
                 "\tjmp \tcontinue%d\n"
                 "\tpush\t0\n"
                 "\tlabel%d:\n",
                 nlabel, ncontinue, nlabel);
    
    // write the left part of the expression
    write_tree(globals, collection, fun, SECONDCHILD(tree));

    fprintf(out, "\tcontinue%d:\n", ncontinue);
    write_bool_transform();
}

static void write_neg(const Table* globals, const FunctionCollection* collection,
                      const Function* fun, const Node* tree) {
    int ncontinue = next_free_label();
    int nlabel = next_free_label();

    fprintf(out, "\n\t; evaluation d'un 'non' (!)\n"
                 "\t; label%d -> si 0 on met a 1\n"
                 "\t; continue%d -> suite du code\n",
                 nlabel, ncontinue);

    write_tree(globals, collection, fun, FIRSTCHILD(tree));

    fprintf(out, "\n\t; evaluation du 'non' (!)\n"
                 "\tpop \trax\n"
                 "\tcmp \trax, 0\n"
                 "\tje  \tlabel%d\n"
                 "\tpush\t0\n"
                 "\tjmp \tcontinue%d\n"
                 "\tlabel%d:\n"
                 "\tpush\t1\n"
                 "\tcontinue%d:\n",
                 nlabel, ncontinue, nlabel, ncontinue);
}

static void write_if(const Table* globals, const FunctionCollection* collection,
                     const Function* fun, const Node* tree) {
    int ncontinue = next_free_label();
    int nelse = next_free_label();

    fprintf(out, "\n\t; evaluation d'un 'if'\n"
                 "\t; continue%d -> code apres la condition\n"
                 "\t; else%d -> code du else\n", 
                 ncontinue, nelse);
    
    // evaluate condition
    write_tree(globals, collection, fun, FIRSTCHILD(tree));

    fprintf(out, "\n\t; evaluation de la condition du if\n"
                 "\tpop \trax\n"
                 "\tcmp \trax, 0\n"
                 "\tje  \telse%d\n",
                 nelse);
    
    // instruction inside the if
    write_tree(globals, collection, fun, SECONDCHILD(tree));
    
    fprintf(out, "\tjmp \tcontinue%d\n"
                 "\telse%d:\n", ncontinue, nelse);

    // instruction inside the else
    write_instructions(globals, collection, fun, THIRDCHILD(tree));

    fprintf(out, "\tcontinue%d:\n", ncontinue);
}

static void write_while(const Table* globals, const FunctionCollection* collection,
                        const Function* fun, const Node* tree) {
    int ncontinue = next_free_label();
    int nhead = next_free_label();

    fprintf(out, "\n\t; evaluation d'un 'while'\n"
                 "\t; continue%d -> code apres un while\n"
                 "\t; head%d -> tete de boucle\n"
                 "\thead%d:\n",
                 ncontinue, nhead, nhead);

    // evaluate condition
    write_tree(globals, collection, fun, FIRSTCHILD(tree));

    fprintf(out, "\n\t; evaluation de la condition du while\n"
                 "\tpop \trax\n"
                 "\tcmp \trax, 0\n"
                 "\tje  \tcontinue%d\t;\n",
                 ncontinue);
    
    // write while code
    write_instructions(globals, collection, fun, SECONDCHILD(tree));

    fprintf(out, "\tjmp \thead%d\n"
                 "\tcontinue%d:\n",
                 nhead, ncontinue);
}

static void write_num(const Node* tree) {
    fprintf(out, "\n\t; lecture d'entier\n"
                 "\tpush\t%d\n",
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
    } else if (!strcmp(tree->val.ident, "'\\''")) {
        sym = '\'';
    } else if (!strcmp(tree->val.ident, "'\\0'")) {
        sym = '\0';
    }

    if (sym == -1) {
        fprintf(out, "\n\t; lecture de charactere\n"
                     "\tpush\t%s\n",
                     tree->val.ident);
    } else {
        fprintf(out, "\n\t; lecture de charactere\n"
                     "\tpush\t%d\n",
                     sym);
    }

}

static void write_tree(const Table* globals, const FunctionCollection* collection,
                       const Function* fun, const Node* tree) {
    if (!tree) return;
    
    switch (tree->label) {
        case SuiteInstr: write_instructions(globals, collection, fun, FIRSTCHILD(tree)); return;
        case Assignation: write_assign(globals, collection, fun, tree); return;
        case Ident: write_load_ident(globals, collection, fun, tree); return;
        case Num: write_num(tree); return;
        case Character: write_character(tree); return;
        case DivStar:
        case AddSub: write_arithmetic(globals, collection, fun, tree); return;
        case Return: write_return(globals, collection, fun, tree); return;
        case Order:
        case Eq: write_comp(globals, collection, fun, tree); return;
        case And: write_and(globals, collection, fun, tree); return;
        case Or: write_or(globals, collection, fun, tree); return;
        case Negation: write_neg(globals, collection, fun, tree); return;
        case If: write_if(globals, collection, fun, tree); return;
        case Else: write_instructions(globals, collection, fun, FIRSTCHILD(tree)); return;
        case While: write_while(globals, collection, fun, tree); return;
        default: return;
    }
}

static void write_instructions(const Table* globals, const FunctionCollection* collection,
                               const Function* fun, const Node* tree) {
    if (!tree) return;
    if (tree->label == SuiteInstr) {
        tree = FIRSTCHILD(tree);
    }

    for (; tree;) {
        write_tree(globals, collection, fun, tree);
        tree = tree->nextSibling;
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

        write_instructions(globals, collection, fun, head_instr);
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
