#include "table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "errors.h"
#include "types.h"

#define NB_BUILTIN 4

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
static int insert_entry(Table* table, Entry entry, bool update_adress);

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
static int decl_var(Table* table, FunctionCollection* coll, t_type type, Node* node, Table* parameters);

/**
 * @brief Initialise a collection of variables of differents types
 * 
 * @param table table to store the variables
 * @param node head node
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int decl_vars(Table* table, FunctionCollection* coll, Node* node, Table* parameters);

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
    int size = 8;               // size of int and char is 8 bytes 
    int additionnal = 1;
    if (is_array(node->type) && FIRSTCHILD(node)) {
        additionnal = FIRSTCHILD(node)->val.num;
    }
    return size*additionnal;
}

static int init_entry(Entry* entry, t_type type, Node* node) {
    entry->decl_line = node->lineno;
    entry->decl_col = node->colno;
    entry->is_used = false;
    entry->type = set_type(type, node->type);
    strcpy(entry->name, node->val.ident);

    entry->size = compute_size(type, node);
    if (!entry->size) {
        incorrect_array_decl(entry->name, node->lineno, node->colno);
        return 0;
    }

    // address will be known when inserting the entry in a table
    entry->address = -1;
    return 1;
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

static int insert_entry(Table* table, Entry entry, bool update_adress) {
    if (!table) return 0;
    int index;
    if ((index = is_in_table(table, entry.name)) != -1) {
        already_declared_error(entry.name, entry.decl_line, entry.decl_col,
                               table->array[index].decl_col);
        return 0;
    }

    if (table->cur_len == table->max_len) {
        if (!realloc_table(table)) return 0;
    }

    if (update_adress) {
        entry.address = table->total_bytes;
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
    
    t_type type = get_type(node->val.ident);
    Entry entry;
    if (!init_entry(&entry, type, FIRSTCHILD(node))) {
        return 0;
    }
    if (!insert_entry(table, entry, false)) {
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
        return 0;
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
        return 0;
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

static int decl_var(Table* table, FunctionCollection* coll, t_type type, Node* node, Table* parameters) {
    if (!node) {
        return 1;
    }

    Entry entry;
    int index;
    if (!init_entry(&entry, type, node)) {
        return 0;
    }
    
    if (parameters) { // locals
        if ((index = is_in_table(parameters, entry.name)) != -1) {
            already_declared_error(entry.name, entry.decl_line, entry.decl_col,
                                parameters->array[index].decl_line);
            return 0;
        }
    } else { // globals
        if ((index = is_in_collection(coll, entry.name)) != -1) {
            redefinition_of_builtin_functions(entry.name, entry.decl_line,
                                              entry.decl_col);
            return 0;
        }
    }

    if (!insert_entry(table, entry, true)) {
        return 0;
    }
    return decl_var(table, coll, type, node->nextSibling, parameters);
}

static t_type get_type(char ident[IDENT_LEN]) {
    if (!strcmp("int", ident)) return T_INT;
    return T_CHAR; 
}

static int decl_vars(Table* table, FunctionCollection* coll, Node* node, Table* parameters) {
    if (!node) {
        return 1;
    }
    t_type type = get_type(node->val.ident);
    if (!decl_var(table, coll, type, FIRSTCHILD(node), parameters)) {
        return 0;
    }
    return decl_vars(table, coll, node->nextSibling, parameters);
}

static int check_used(Table* globals, Function* fun, FunctionCollection* coll, Node* node) {
    if (!node) return 1;

    if (node->label == Ident) {
        Entry* entry = find_entry(globals, fun, node->val.ident);
        if (!entry) {
            Function* f = get_function(coll, node->val.ident);
            if (!f) {
                use_of_undeclare_symbol(node->val.ident, node->lineno, node->colno);
                return 0;
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
    if (!check_used(globals, fun, coll, FIRSTCHILD(node))) return 0;
    return check_used(globals, fun, coll, node->nextSibling);
}

static int create_builtin_function(Function* fun, builtin spe) {
    *fun = (Function){.decl_col = -1,
                      .decl_line = -1,
                      .is_used = false,
                      .r_type = spe.r_type
                      };
    strcpy(fun->name, spe.name);
    if (!init_table(&fun->parameters)) {
        return 0;
    }
    if (spe.param != T_VOID) {
        Entry entry = (Entry){.address = -1,
                              .decl_col = -1,
                              .decl_line = -1,
                              .is_used = true,
                              .name = "arg",
                              .size = 8,
                              .type = spe.param};
        if (!insert_entry(&fun->parameters, entry, false)) {
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
    static bool is_globals_done = false;

    if (!node) {
        return 1;
    }

    int err;
    if (node->label == DeclVars) {
        if (!is_globals_done) {
            err = decl_vars(globals, collection, FIRSTCHILD(node), NULL);
            is_globals_done = true;
        } else {
            err = decl_vars(&(collection->funcs[collection->cur_len - 1].locals),
                            collection, FIRSTCHILD(node),
                            &(collection->funcs[collection->cur_len - 1].parameters));
        }
        if (!err) return 0;
        return create_tables(globals, collection, node->nextSibling);
    } else if (node->label == DeclFonct) {
        Function fun;
        if (!init_function(&fun, FIRSTCHILD(FIRSTCHILD(node)), globals)) {
            return 0;
        }

        Node* head_decl_vars = FIRSTCHILD(FIRSTCHILD(SECONDCHILD(node)));

        if (!decl_vars(&fun.locals, collection, head_decl_vars, &fun.parameters)) {
            return 0;
        }
        
        if (!insert_function(collection, fun)) {
            return 0;
        }
        
        if (!check_used(globals, &fun, collection, SECONDCHILD(node))) {
            return 0;
        }
        
        return create_tables(globals, collection, node->nextSibling);
    }
    if (!create_tables(globals, collection, FIRSTCHILD(node))) {
        return 0;
    }
    return create_tables(globals, collection, node->nextSibling);
}

void print_table(Table table) {
    for (int i = 0; i < table.cur_len; i++) {
        if (table.array[i].address >= 0) {
            printf("type: %4s | decl_line: %3d | size: %5d | address: %05xx | array: %s | name: %s\n",
                table.array[i].type == T_INT ? "int": "char",
                table.array[i].decl_line, 
                table.array[i].size,
                table.array[i].address,
                is_array(table.array[i].type) ? "true": "false",
                table.array[i].name);
        } else {
            printf("type: %4s | decl_line: %3d | size: %5d | array: %s | name: %s\n",
                table.array[i].type == T_INT ? "int": "char",
                table.array[i].decl_line, 
                table.array[i].size,
                is_array(table.array[i].type) ? "true": "false",
                table.array[i].name);
        }

    }
}

void print_collection(FunctionCollection collection) {
    for (int i = 0; i < collection.cur_len; i++) {
        if (collection.funcs[i].decl_line == -1) {
            continue; // do not print builtin functions
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
