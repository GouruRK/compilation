#include "table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "errors.h"

#define NB_BUILTIN 4

typedef struct {
    char* name;
    Types r_type;
    Types param;
} builtin;

static const builtin builtin_funcs[NB_BUILTIN] = {
    {.name = "getint",  .r_type = T_INT,  .param = T_VOID},
    {.name = "putint",  .r_type = T_VOID, .param = T_INT},
    {.name = "getchar", .r_type = T_CHAR, .param = T_VOID},
    {.name = "putchar", .r_type = T_VOID, .param = T_CHAR}
};

int total_bytes = 0;

/**
 * @brief Compute size of variable in bytes based on its type
 * 
 * @param type value type
 * @param node declaration node
 * @return size in bytes
 */
static int compute_size(Types type, Node* node);

/**
 * @brief Create an entry strucutre that gives intels about a variable
 * 
 * @param type type of variable
 * @param node node contains the variable name
 * @return created entry
 */
static Entry init_entry(Types type, Node* node, int last_address);

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
static int insert_entry(Table* table, Entry entry);

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
static int decl_var(Table* table, Types type, Node* node, Table* parameters);

/**
 * @brief Initialise a collection of variables of differents types
 * 
 * @param table table to store the variables
 * @param node head node
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int decl_vars(Table* table, Node* node, Table* parameters);

/**
 * @brief Get the type object from its identifiant
 * 
 * @param ident type identifiant
 * @return type
 */
static Types get_type(char ident[IDENT_LEN]);

/**
 * @brief Initialise a collection of variables of differents types
 * 
 * @param table table to store the variables
 * @param node head node
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int decl_vars(Table* table, Node* node, Table* parameters);

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

static int compute_size(Types type, Node* node) {
    int size = 8;               // size of int and char is 8 bytes 
    int additionnal = 1;
    if (node->array && node->firstChild) {
        additionnal = node->firstChild->val.num;
    }
    return size*additionnal;
}

static Entry init_entry(Types type, Node* node, int last_address) {
    Entry entry;
    entry.array = node->array;
    entry.decl_line = node->lineno;
    entry.decl_col = node->colno;
    entry.is_used = false;
    entry.size = compute_size(type, node);
    entry.type = type;
    strcpy(entry.name, node->val.ident);

    if (last_address < 0) {
        entry.address = -1;
    } else {
        entry.address = total_bytes;
        total_bytes += entry.size;
    }
    return entry;
}

static int realloc_table(Table* table) {
    int next_len = table->max_len + DEFAULT_LENGTH;

    Entry* temp = realloc(table->array, sizeof(Entry)*next_len);
    if (!temp) {
        memory_error();
        return 0;
    }
    table->array = temp;
    table->max_len = next_len;
    return 1;
}

static int insert_entry(Table* table, Entry entry) {
    if (!table) return 0;
    int index;
    if ((index = is_in_table(table, entry.name)) != -1) {
        already_declared_error(entry.name, entry.decl_line,
                       entry.decl_col, table->array[index].decl_col);
        return 1;
    }

    if (table->cur_len == table->max_len) {
        if (!realloc_table(table)) return 0;
    }

    table->array[table->cur_len] = entry;
    table->total_bytes += entry.size;
    table->cur_len++;
    return 1;
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
        return 1;
    }
    
    Types type = get_type(node->val.ident);
    if (!insert_entry(table, init_entry(type, node->firstChild, -1))) {
        return 0;
    }
    init_param_list(table, node->nextSibling);
    return 1;
}

static int init_function(Function* fun, Node* node, Table* globals) {
    fun->is_used = false;
    fun->decl_line = node->lineno;
    fun->decl_col = node->colno;
    assing_rtype(fun, node);
    strcpy(fun->name, node->nextSibling->val.ident);

    int index;
    if ((index = is_in_table(globals, fun->name)) != -1) {  
        already_declared_error(fun->name, fun->decl_line, fun->decl_col,
                                   globals->array[index].decl_line);
    }

    if (!init_table(&fun->parameters)) {
        return 0;
    }
    if (!init_table(&fun->locals)) {
        free(fun->parameters.array);
        return 0;
    }
    if (node->nextSibling->nextSibling->label == ListTypVar) {
        return init_param_list(&fun->parameters,
                               node->nextSibling->nextSibling->firstChild);
    }
    return 1;
}

static int realloc_collection(FunctionCollection* collection) {
    int next_len = collection->max_len + DEFAULT_LENGTH;

    Function* temp = (Function*)realloc(collection->funcs, sizeof(Function)*next_len);
    if (!temp) {
        memory_error();
        return 0;
    }
    collection->funcs = temp;
    collection->max_len = next_len;
    return 1;
}

static int insert_function(FunctionCollection* collection, Function fun) {
    if (!collection) return 0;
    int index;
    if ((index = is_in_collection(collection, fun.name)) != -1) {
        already_declared_error(fun.name, fun.decl_line, fun.decl_col,
                                   collection->funcs[index].decl_line);
        return 1; // return 1 to continue
    }

    if (collection->cur_len == collection->max_len) {
        if (!realloc_collection(collection)) {
            return 0;
        }
    }

    collection->funcs[collection->cur_len] = fun;
    collection->cur_len++;
    return 1;
}

static int decl_var(Table* table, Types type, Node* node, Table* parameters) {
    if (!node) {
        return 1;
    }

    Entry entry = init_entry(type, node, total_bytes);
    int index;

    if (parameters && (index = is_in_table(parameters, entry.name)) != -1) {
        already_declared_error(entry.name, entry.decl_line,
                       entry.decl_col, parameters->array[index].decl_line);
        return 1;
    }

    if (!insert_entry(table, entry)) {
        return 0;
    }
    return decl_var(table, type, node->nextSibling, parameters);
}

static Types get_type(char ident[IDENT_LEN]) {
    if (!strcmp("int", ident)) return T_INT;
    return T_CHAR; 
}

static int decl_vars(Table* table, Node* node, Table* parameters) {
    if (!node) {
        return 1;
    }
    Types type = get_type(node->val.ident);
    if (!decl_var(table, type, node->firstChild, parameters)) {
        return 0;
    }
    return decl_vars(table, node->nextSibling, parameters);
}

static void check_used(Table* globals, Function* fun, FunctionCollection* coll, Node* node) {
    if (!node) return;

    if (node->label == Ident) {
        Entry* entry = find_entry(globals, fun, node->val.ident);
        if (!entry) {
            Function* f = get_function(coll, node->val.ident);
            if (!f) {
                use_of_undeclare_symbol(node->val.ident, node->lineno, node->colno);
                return;
            } else {
                if (f->decl_line != node->lineno) {
                    f->is_used = true;
                }
            }
        } else {
            if (entry->decl_line != node->lineno) {
                entry->is_used = true;
            }
        }
    }
    check_used(globals, fun, coll, node->firstChild);
    check_used(globals, fun, coll, node->nextSibling);
}

static int create_builtin_function(Function* fun, builtin spe) {
    *fun = (Function){.decl_col = -1,
                      .decl_line = -1,
                      .is_used = true,
                      .r_type = spe.r_type
                      };
    strcpy(fun->name, spe.name);
    if (!init_table(&fun->parameters)) {
        return 0;
    }
    if (spe.param != T_VOID) {
        Entry entry = (Entry){.address = -1,
                              .array = false,
                              .decl_col = -1,
                              .decl_line = -1,
                              .is_used = true,
                              .name = "arg",
                              .size = 8,
                              .type = spe.param};
        if (!insert_entry(&fun->parameters, entry)) {
            free_table(&fun->parameters);
            return 0;
        }
    }
    if (!init_table(&fun->locals)) {
        free_table(&fun->parameters);
        return 0;
    }
    return 1;
}

static int insert_builtin_functions(FunctionCollection* coll) {
    for (int i = 0; i < NB_BUILTIN; i++) {
        Function fun;
        if (!create_builtin_function(&fun, builtin_funcs[i])) {
            return 0;
        }
        if (!insert_function(coll, fun)) {
            return 0;
        }
    }
    return 1;
}

int init_table(Table* table) {
    if (!table) return 0;

    table->total_bytes = 0;
    table->sorted = false;
    table->cur_len = 0;
    table->array = (Entry*)malloc(sizeof(Entry)*DEFAULT_LENGTH);

    if (!table->array) {
        memory_error();
        table->max_len = 0;
        return 0;
    }
    table->max_len = DEFAULT_LENGTH;
    return 1;
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

Entry* find_entry(const Table* globals, const Function* fun, const char ident[IDENT_LEN]) {
    Entry* entry;
    if ((entry = get_entry(&fun->parameters, ident))) return entry;
    if ((entry = get_entry(&fun->locals, ident))) return entry;
    if ((entry = get_entry(globals, ident))) return entry;
    return NULL;
}

Entry* get_entry(const Table* table, const char ident[IDENT_LEN]) {
    if (!table || !table->cur_len) return NULL;

    if (table->sorted) {
        return bsearch(ident, table->array, table->cur_len, sizeof(Entry),
        compare_ident_entry);
    } 
    int index = is_in_table(table, ident);
    return index == -1 ? NULL: &(table->array[index]);
}

int init_function_collection(FunctionCollection* collection) {
    if (!collection) return 0;

    collection->sorted = false;
    collection->cur_len = 0;
    collection->funcs = (Function*)malloc(sizeof(Function)*DEFAULT_LENGTH);

    if (!collection->funcs) {
        memory_error();
        collection->max_len = 0;
        return 0;
    }
    collection->max_len = DEFAULT_LENGTH;
    if (!insert_builtin_functions(collection)) {
        memory_error();
        free_collection(collection);
        return 0;
    }
    return 1;
}

int is_in_collection(const FunctionCollection* collection, const char ident[IDENT_LEN]) {
    if (!collection || !collection->cur_len) return -1;

    for (int i = 0; i < collection->cur_len; i++) {
        if (!strcmp(collection->funcs[i].name, ident)) {
            return i;
        }
    }
    return -1;
}

Function* get_function(const FunctionCollection* collection, const char ident[IDENT_LEN]) {
    if (!collection || !collection->cur_len) return NULL;
    
    if (collection->sorted) {
        return bsearch(ident, collection->funcs, collection->cur_len, sizeof(Function),
                       compare_ident_fun);
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
    static bool is_globals_done = false;

    if (!node) {
        return 1;
    }

    int err;
    if (node->label == DeclVars) {
        if (!is_globals_done) {
            err = decl_vars(globals, node->firstChild, NULL);
            is_globals_done = true;
        } else {
            err = decl_vars(&(collection->funcs[collection->cur_len - 1].locals),
                            node->firstChild,
                            &(collection->funcs[collection->cur_len - 1].parameters));
        }
        if (!err) return 0;
        return create_tables(globals, collection, node->nextSibling);
    } else if (node->label == DeclFonct) {
        Function fun;
        if (!init_function(&fun, node->firstChild->firstChild, globals)) {
            return 0;
        }

        Node* head_decl_vars = node->firstChild->nextSibling->firstChild->firstChild;

        if (!decl_vars(&fun.locals, head_decl_vars, &fun.parameters)) {
            return 0;
        }
        
        if (!insert_function(collection, fun)) {
            return 0;
        }
        
        check_used(globals, &fun, collection, node->firstChild->nextSibling);
        
        return create_tables(globals, collection, node->nextSibling);
    }
    if (!create_tables(globals, collection, node->firstChild)) {
        return 0;
    }
    return create_tables(globals, collection, node->nextSibling);
}

void print_table(Table table) {
    for (int i = 0; i < table.cur_len; i++) {
        if (table.array[i].address >= 0) {
            printf("type: %4s | decl_line: %3d | size: %5d | address: %05xx | name: %s\n",
                table.array[i].type == T_INT ? "int": "char",
                table.array[i].decl_line, 
                table.array[i].size,
                table.array[i].address,
                table.array[i].name);
        } else {
            printf("type: %4s | decl_line: %3d | size: %5d | name: %s\n",
                table.array[i].type == T_INT ? "int": "char",
                table.array[i].decl_line, 
                table.array[i].size,
                table.array[i].name);
        }

    }
}

void print_collection(FunctionCollection collection) {
    for (int i = 0; i < collection.cur_len; i++) {
        // if (collection.funcs[i].decl_line == -1) {
        //     continue; // do not print builtin functions
        // }
        Types type = collection.funcs[i].r_type;
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
