#include "table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "errors.h"
#include "types.h"

#define NB_BUILTIN 4
#define CALL_OFFSET 16
#define N_REG_PARAM 6

#define SEM_ERR  0
#define SEM_GOOD 1

typedef struct {        // structure for builtin function
    char* name;         // function name
    t_type r_type;      // return type
    t_type param;       // parameter type
} builtin;

// array of builtin
static const builtin builtin_funcs[NB_BUILTIN] = {
    {.name = "getint",  .r_type = T_INT,  .param = T_VOID},
    {.name = "putint",  .r_type = T_VOID, .param = T_INT},
    {.name = "getchar", .r_type = T_CHAR, .param = T_VOID},
    {.name = "putchar", .r_type = T_VOID, .param = T_CHAR}
};

/**
 * @brief Compute size of variable in bytes based on its type
 * 
 * @param type value type
 * @param node declaration node
 * @return size in bytes
 */
static int compute_size(t_type type, Node* node);

/**
 * @brief Create an entry strucutre that gives intels about a variable
 * 
 * @param type type of variable
 * @param node node contains the variable name
 * @return created entry
 */
static int init_entry(Entry* entry, t_type type, Node* node);

/**
 * @brief Allocate more memory for a table
 * 
 * @param table table to realloc
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int realloc_table(Table* table);

/**
 * @brief Insert an entry in the table
 * 
 * @param table table to insert
 * @param entry entry to be inserted
 * @return 1 if success
 *         0 if error due to memory error
 */
static int insert_entry(Table* table, Entry entry, int new_address);

/**
 * @brief Assign to the given function its return type
 * 
 * @param fun function to assign
 * @param node type node
 */
static void assing_rtype(Function* fun, Node* node);

/**
 * @brief Initiate parameters lists from functions
 * 
 * @param table table to store the parameters
 * @param node head node of the list
 * @return 1 if success
 *         0 if error due to memory error
 */
static int init_param_list(Table* table, Node* node);

/**
 * @brief Create a structure for intels about functions
 * 
 * @param node return type of function
 * @return created function
 */
static int init_function(Function* fun, Node* node, Table* globals);

/**
 * @brief Allocate more memory for a collection
 * 
 * @param collection collection to re alloc
 * @return 1 if success
 *         0 if fail due to memory error 
 */
static int realloc_collection(FunctionCollection* collection);

/**
 * @brief Insert function in collection
 * 
 * @param collection collection to insert
 * @param fun function to insert
 * @return 1 if success
 *         0 if error due to memory error
 */
static int insert_function(FunctionCollection* collection, Function fun);

/**
 * @brief Initialise a variable collection of given type
 * 
 * @param table table to store entries
 * @param type type of the following variables
 * @param node head node
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int decl_var(Table* table, FunctionCollection* coll, t_type type,
                    Node* node, Table* parameters);

/**
 * @brief Initialise a collection of variables of differents types
 * 
 * @param table table to store the variables
 * @param node head node
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int decl_vars(Table* table, FunctionCollection* coll, Node* node,
                     Table* parameters);

/**
 * @brief Get the type object from its identifiant
 * 
 * @param ident type identifiant
 * @return type
 */
static t_type get_type(char ident[IDENT_LEN]);

/**
 * @brief Create a builtin function according to the given specification
 * 
 * @param fun function to create
 * @param spe indications to create the function
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int create_builtin_function(Function* fun, builtin spe);

/**
 * @brief Create and insert all builtin functions
 * 
 * @param coll collection to insert the functions
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int insert_builtin_functions(FunctionCollection* coll);

int compare_entries(const void* entry1, const void* entry2) {
    return strcmp(((Entry*)entry1)->name, ((Entry*)entry2)->name);
}

int compare_functions(const void* fun1, const void* fun2) {
    return strcmp(((Function*)fun1)->name, ((Function*)fun2)->name);
}

int compare_ident_entry(const void* ident, const void* entry) {
    return strcmp((char*)ident, ((Entry*)entry)->name);
}

int compare_ident_fun(const void* ident, const void* fun) {
    return strcmp((char*)ident, ((Function*)fun)->name);
}

static int compute_size(t_type type, Node* node) {
    int size = 8; // size of int and char is on 8 bytes 
    int additionnal = 1;
    if (is_array(node->type) && FIRSTCHILD(node)) {
        additionnal = FIRSTCHILD(node)->val.num;
    }
    return size*additionnal;
}

static int init_entry(Entry* entry, t_type type, Node* node) {
    entry->decl_line = node->lineno;            // set declaration line
    entry->decl_col = node->colno;              // set declaration colunm
    entry->is_used = false;                     // set as unused
    entry->type = set_type(type, node->type);   // set its type
    strcpy(entry->name, node->val.ident);       // set its name

    entry->size = compute_size(type, node);     // variable size
    if (!entry->size) {
        incorrect_array_decl(entry->name, node->lineno, node->colno);
        return SEM_ERR;
    }
    entry->address = -1;                        // address to be known when
                                                // inserting the entry into its
                                                // table
    return SEM_GOOD;
}

static int realloc_table(Table* table) {
    int next_len = table->max_len + DEFAULT_LENGTH;

    Entry* temp = realloc(table->array, sizeof(Entry)*next_len);
    if (!temp) {
        memory_error();
        return SEM_ERR;
    }
    table->array = temp;
    table->max_len = next_len;
    return SEM_GOOD;
}

static int insert_entry(Table* table, Entry entry, int new_address) {
    if (!table) return SEM_ERR;
    int index;

    // check if an entry with the same name is already declared
    if ((index = is_in_table(table, entry.name)) != -1) {
        // trigger a semantic error of type 'already_declared'
        already_declared_error(entry.name, entry.decl_line, entry.decl_col,
                               table->array[index].decl_col);
        return SEM_ERR;
    }

    // if there is no place remaining, realloc the array
    if (table->cur_len == table->max_len) {
        if (!realloc_table(table)) return SEM_ERR;
    }

    // set the entry address
    entry.address = new_address;

    // update table properties
    table->total_bytes += entry.size;
    table->array[table->cur_len] = entry;
    table->cur_len++;
    return SEM_GOOD;
}

static void assing_rtype(Function* fun, Node* node) {
    if (!strcmp(node->val.ident, "int")) {
        fun->r_type = T_INT;
    } else if (!strcmp(node->val.ident, "char")) {
        fun->r_type = T_CHAR;
    } else {
        fun->r_type = T_VOID;
    }
}

static int init_param_list(Table* table, Node* node) {
    if (!node) {
        return SEM_GOOD;
    }
    
    int new_address;
    Entry entry;
    t_type type = get_type(node->val.ident);
    if (!init_entry(&entry, type, FIRSTCHILD(node))) {
        return SEM_ERR;
    }

    // According to AMD64 call convetions in nasm, the first 6-th parameters
    // must be given to function using 6 registers. Any other parameters must be
    // given using the stack.
    // Parameters address's change to adapt them if they are before or after
    // the stack saving register 'rbp'. When a function is called, there is
    // a 16 bytes offset 'CALL_OFFSET'

    if (table->cur_len < N_REG_PARAM) {
        // parameters 1 to 6
        new_address = table->total_bytes + entry.size;
        table->offset += entry.size;
    } else if (table->cur_len == N_REG_PARAM) {
        // 6-th parameter
        new_address = CALL_OFFSET;
    } else {
        // 7-th parameter and go on
        new_address = table->array[table->cur_len - 1].address + entry.size;
    }

    if (!insert_entry(table, entry, new_address)) {
        return SEM_ERR;
    }
    
    // initiate the next parameter
    init_param_list(table, node->nextSibling);
    return SEM_GOOD;
}

static int init_function(Function* fun, Node* node, Table* globals) {
    fun->is_used = false;                             // set function as unsused
    fun->decl_line = node->lineno;                    // set declaration line
    fun->decl_col = node->colno;                      // set declaration column
    assing_rtype(fun, node);                          // set return type
    strcpy(fun->name, node->nextSibling->val.ident);  // set name

    int index;
    // check if a function or a global variable with the same name is already 
    // declared
    if ((index = is_in_table(globals, fun->name)) != -1) {
        // trigger a semantic error
        already_declared_error(fun->name, fun->decl_line, fun->decl_col,
                               globals->array[index].decl_line);
        return SEM_ERR;
    }

    // create table to store parameter entries
    if (!init_table(&fun->parameters)) {
        // memory error while creating the parameters table
        return SEM_ERR;
    }
    if (node->nextSibling->nextSibling->label == ListTypVar) {
        // insert parameter entries
        if (!init_param_list(&fun->parameters,
                             node->nextSibling->nextSibling->firstChild)) {
            free(&fun->parameters);
            return SEM_ERR;
        }
    }

    // create table to store local entries
    if (!init_table(&fun->locals)) {
        free(fun->parameters.array);
        return SEM_ERR;
    }

    // add an offset of 8 to prevent going on the wrong stack address
    fun->locals.total_bytes = 8;
    return SEM_GOOD;
}

static int realloc_collection(FunctionCollection* collection) {
    int next_len = collection->max_len + DEFAULT_LENGTH;

    Function* temp = (Function*)realloc(collection->funcs,
                                        sizeof(Function)*next_len);
    if (!temp) {
        memory_error();
        return SEM_ERR;
    }

    // update the collection's data 
    collection->funcs = temp;
    collection->max_len = next_len;
    return SEM_GOOD;
}

static int insert_function(FunctionCollection* collection, Function fun) {
    if (!collection) return SEM_ERR;
    int index;
    // check if a function with the same name is already declared
    if ((index = is_in_collection(collection, fun.name)) != -1) {
        // trigger a semantic error
        already_declared_error(fun.name, fun.decl_line, fun.decl_col,
                               collection->funcs[index].decl_line);
        return SEM_ERR;
    }

    if (collection->cur_len == collection->max_len) {
        if (!realloc_collection(collection)) {
            return SEM_ERR;
        }
    }

    // update collection's data
    collection->funcs[collection->cur_len] = fun;
    collection->cur_len++;
    return SEM_GOOD;
}

static int decl_var(Table* table, FunctionCollection* coll, t_type type,
                    Node* node, Table* parameters) {
    if (!node) {
        return SEM_GOOD;
    }

    Entry entry;
    int index;
    if (!init_entry(&entry, type, node)) {
        return SEM_ERR;
    }

    // declare a local variable
    if (parameters) {
        // check if the local entry is already declared
        if ((index = is_in_table(parameters, entry.name)) != -1) {
            // trigger a semantic error
            already_declared_error(entry.name, entry.decl_line, entry.decl_col,
                                parameters->array[index].decl_line);
            return SEM_ERR;
        }
        if (!insert_entry(table, entry, table->total_bytes)) {
            return SEM_ERR;
        }
    } else { // declare a global variable
        // check if the global variable is already declared
        if ((index = is_in_collection(coll, entry.name)) != -1) {
            // trigger a semantic error
            redefinition_of_builtin_functions(entry.name, entry.decl_line,
                                              entry.decl_col);
            return SEM_ERR;
        }
        if (!insert_entry(table, entry, table->total_bytes)) {
            return SEM_ERR;
        }
    }
    return decl_var(table, coll, type, node->nextSibling, parameters);
}

static t_type get_type(char ident[IDENT_LEN]) {
    if (!strcmp("int", ident)) return T_INT;
    return T_CHAR; 
}

static int decl_vars(Table* table, FunctionCollection* coll, Node* node,
                     Table* parameters) {
    if (!node) {
        return SEM_GOOD;
    }
    t_type type = get_type(node->val.ident);
    if (!decl_var(table, coll, type, FIRSTCHILD(node), parameters)) {
        return SEM_ERR;
    }
    return decl_vars(table, coll, node->nextSibling, parameters);
}

static int check_used(Table* globals, Function* fun, FunctionCollection* coll,
                      Node* node) {
    if (!node) return SEM_GOOD;

    if (node->label == Ident) {
        // check if there is a child, so ident can whenever be an array or a function
        if (FIRSTCHILD(node)) {
            // definitively a function
            if (FIRSTCHILD(node)->label == NoParametres || FIRSTCHILD(node)->label == ListExp) {
                Function* f = get_function(coll, node->val.ident);
                if (!f) {
                    use_of_undeclare_symbol(WARNING, node->val.ident,
                                            node->lineno, node->colno);
                } else {
                    if (f->decl_line != node->lineno) {
                        f->is_used = true;
                    }
                }
                return SEM_GOOD;
            }
        }
        Entry* entry = find_entry(globals, fun, node->val.ident);
        if (!entry) {
            use_of_undeclare_symbol(ERROR, node->val.ident, node->lineno,
                                    node->colno);
            return SEM_ERR;
        }
        if (entry->decl_line != node->lineno) {
            entry->is_used = true;
        }
    }
    if (!check_used(globals, fun, coll, FIRSTCHILD(node))) return SEM_ERR;
    return check_used(globals, fun, coll, node->nextSibling);
}

static int create_builtin_function(Function* fun, builtin spe) {
    // set function default value
    *fun = (Function){.decl_col = -1,
                      .decl_line = -1,
                      .is_used = false,
                      .r_type = spe.r_type
                      };
    strcpy(fun->name, spe.name);
    if (!init_table(&fun->parameters)) {
        return SEM_ERR;
    }
    if (spe.param != T_VOID) {
        // set parameter default value
        Entry entry = (Entry){.address = -1,
                              .decl_col = -1,
                              .decl_line = -1,
                              .is_used = true,
                              .name = "arg",
                              .size = 8,
                              .type = spe.param};
        if (!insert_entry(&fun->parameters, entry, 0)) {
            free_table(&fun->parameters);
            return SEM_ERR;
        }
    }
    if (!init_table(&fun->locals)) {
        free_table(&fun->parameters);
        return SEM_ERR;
    }
    return SEM_GOOD;
}

static int insert_builtin_functions(FunctionCollection* coll) {
    for (int i = 0; i < NB_BUILTIN; i++) {
        Function fun;
        if (!create_builtin_function(&fun, builtin_funcs[i])) {
            return SEM_ERR;
        }
        if (!insert_function(coll, fun)) {
            return SEM_ERR;
        }
    }
    return SEM_GOOD;
}

int init_table(Table* table) {
    if (!table) return SEM_ERR;

    // set table default values
    table->total_bytes = 0;
    table->sorted = false;
    table->cur_len = 0;
    table->offset = 0;

    table->array = (Entry*)malloc(sizeof(Entry)*DEFAULT_LENGTH);
    if (!table->array) {
        memory_error();
        table->max_len = 0;
        return SEM_ERR;
    }
    table->max_len = DEFAULT_LENGTH;
    return SEM_GOOD;
}

int is_in_table(const Table* table, const char ident[IDENT_LEN]) {
    if (!table || !table->cur_len) return -1;

    for (int i = 0; i < table->cur_len; i++) {
        if (!strcmp(table->array[i].name, ident)) {
            return i;
        }
    }
    return -1;
}

Entry* find_entry(const Table* globals, const Function* fun,
                  const char ident[IDENT_LEN]) {
    Entry* entry;

    // check if the entry is known as a parameter, a local or a global variable
    if ((entry = get_entry(&fun->parameters, ident))) return entry;
    if ((entry = get_entry(&fun->locals, ident))) return entry;
    if ((entry = get_entry(globals, ident))) return entry;
    return NULL;
}

Entry* get_entry(const Table* table, const char ident[IDENT_LEN]) {
    if (!table || !table->cur_len) return NULL;

    // choose the fastest method to find an entry
    if (table->sorted) {
        // uses stdlib bsearch function on lexical order
        return bsearch(ident, table->array, table->cur_len, sizeof(Entry),
                       compare_ident_entry);
    } 
    int index = is_in_table(table, ident);
    return index == -1 ? NULL: &(table->array[index]);
}

int init_function_collection(FunctionCollection* collection) {
    if (!collection) return SEM_ERR;

    // collection default values
    collection->sorted = false;
    collection->cur_len = 0;

    collection->funcs = (Function*)malloc(sizeof(Function)*DEFAULT_LENGTH);
    if (!collection->funcs) {
        memory_error();
        collection->max_len = 0;
        return SEM_ERR;
    }
    collection->max_len = DEFAULT_LENGTH;
    
    if (!insert_builtin_functions(collection)) {
        memory_error();
        free_collection(collection);
        return SEM_ERR;
    }
    return SEM_GOOD;
}

int is_in_collection(const FunctionCollection* collection,
                     const char ident[IDENT_LEN]) {
    if (!collection || !collection->cur_len) return -1;

    for (int i = 0; i < collection->cur_len; i++) {
        if (!strcmp(collection->funcs[i].name, ident)) {
            return i;
        }
    }
    return -1;
}

Function* get_function(const FunctionCollection* collection,
                       const char ident[IDENT_LEN]) {
    if (!collection || !collection->cur_len) return NULL;
    
    // choose the fastest method to find an entry
    if (collection->sorted) {
        // uses stdlib bsearch function on lexical order
        return bsearch(ident, collection->funcs, collection->cur_len,
                       sizeof(Function), compare_ident_fun);
    }
    int index = is_in_collection(collection, ident);
    return index == -1 ? NULL: &(collection->funcs[index]);
}

void free_table(Table* table) {
    if (!table) return;
    free(table->array); 
}

void free_collection(FunctionCollection* collection) {
    if (!collection) return;

    for (int i = 0; i < collection->cur_len; i++) {
        free(collection->funcs[i].parameters.array);
        free(collection->funcs[i].locals.array);
    }
    free(collection->funcs);
}

int create_tables(Table* globals, FunctionCollection* collection, Node* node) {
    // main function to create symbol tables
    static bool is_globals_done = false;
    if (!node) {
        return SEM_GOOD;
    }

    int err;
    // declaration of variables
    if (node->label == DeclVars) {
        // declaration of global variables
        if (!is_globals_done) {
            err = decl_vars(globals, collection, FIRSTCHILD(node), NULL);
            is_globals_done = true;
        } else {
            err = decl_vars(&(collection->funcs[collection->cur_len - 1].locals),
                            collection, FIRSTCHILD(node),
                            &(collection->funcs[collection->cur_len - 1].parameters));
        }
        if (!err) return SEM_ERR;
        return create_tables(globals, collection, node->nextSibling);
    } else if (node->label == DeclFonct) { // declaration of function
        Function fun;
        if (!init_function(&fun, FIRSTCHILD(FIRSTCHILD(node)), globals)) {
            return SEM_ERR;
        }

        Node* head_decl_vars = FIRSTCHILD(FIRSTCHILD(SECONDCHILD(node)));
        // insert local entries
        if (!decl_vars(&fun.locals, collection, head_decl_vars, &fun.parameters)) {
            return SEM_ERR;
        }
        // insert function in collection
        if (!insert_function(collection, fun)) {
            return SEM_ERR;
        }
        // check if any of the variables are defined before being use
        if (!check_used(globals, &fun, collection, SECONDCHILD(node))) {
            return SEM_ERR;
        }
        return create_tables(globals, collection, node->nextSibling);
    }
    // continue to parse the tree
    return create_tables(globals, collection, FIRSTCHILD(node)) &&
           create_tables(globals, collection, node->nextSibling);
}

void print_table(Table table) {
    for (int i = 0; i < table.cur_len; i++) {
        printf("type: %4s | decl_line: %3d | size: %5d | array: %s | name: %s\n",
            table.array[i].type == T_INT ? "int": "char",
            table.array[i].decl_line, 
            table.array[i].size,
            is_array(table.array[i].type) ? "true": "false",
            table.array[i].name);
    }
}

void print_collection(FunctionCollection collection) {
    for (int i = 0; i < collection.cur_len; i++) {
        // do not print builtin functions
        if (collection.funcs[i].decl_line == -1) {
            continue;
        }
        t_type type = collection.funcs[i].r_type;
        putchar('\n'); 
        
        printf("%s %s() - Parameters:\n",
                type == T_INT ? "int" : (type == T_CHAR ? "char": "void"),
                collection.funcs[i].name);
        
        print_table(collection.funcs[i].parameters);

        printf("%s %s() - Locals:\n",
                type == T_INT ? "int" : (type == T_CHAR ? "char": "void"),
                collection.funcs[i].name);
        print_table(collection.funcs[i].locals);
    }
}
