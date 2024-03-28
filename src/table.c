#include "table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "error.h"

/**
 * @brief Compute size of variable in bytes based on its type
 * 
 * @param type value type
 * @param node declaration node
 * @return size in bytes
 */
static int compute_size(ValueType type, Node* node) {
    int size = 0;
    int additionnal = 1;
    if (type == NUMERIC) {
        size = S_INT;
    } else if (type == CHAR) {
        size = S_CHAR;
    }
    if (node->array && node->firstChild) {
        additionnal = node->firstChild->val.num;
    }
    return size*additionnal;
}

/**
 * @brief Create an entry strucutre that gives intels about a variable
 * 
 * @param type type of variable
 * @param node node contains the variable name
 * @return created entry
 */
static Entry init_entry(ValueType type, Node* node) {
    Entry entry;
    entry.array = node->array;
    entry.decl_line = node->lineno;
    entry.is_used = false;
    entry.size = compute_size(type, node);
    entry.type = type;
    strcpy(entry.name, node->val.ident);
    return entry;
}

/**
 * @brief Allocate more memory for a table
 * 
 * @param table table to realloc
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int realloc_table(Table* table) {
    int next_len = table->max_len + DEFAULT_LENGTH;

    Entry* temp = realloc(table->array, sizeof(Entry)*next_len);
    if (!temp) {
        add_error(EXCEPTION, MEMORY_ERROR, "reallocation failed");
        return 0;
    }
    table->array = temp;
    table->max_len = next_len;
    return 1;
}

/**
 * @brief Insert an entry in the table
 * 
 * @param table table to insert
 * @param entry entry to be inserted
 * @return 1 if success
 *         0 if error due to memory error
 */
static int insert_entry(Table* table, Entry entry) {
    if (!table) return 0;
    if (is_in_table(table, entry.name) != -1) {
        add_line_error(EXCEPTION, ALREADY_DECLARE, entry.decl_line, entry.name);
        return 1;
    }

    if (table->cur_len == table->max_len) {
        if (!realloc_table(table)) return 0;
    }

    table->array[table->cur_len] = entry;
    table->cur_len++;
    table->total_size += entry.size;
    return 1;
}

/**
 * @brief Assign to the given function its return type
 * 
 * @param fun function to assign
 * @param node type node
 */
static void assing_rtype(Function* fun, Node* node) {
    if (!strcmp(node->val.ident, "int")) {
        fun->r_type = R_INT;
    } else if (!strcmp(node->val.ident, "char")) {
        fun->r_type = R_CHAR;
    } else {
        fun->r_type = R_VOID;
    }
}

/**
 * @brief Initiate parameters lists from functions
 * 
 * @param table table to store the parameters
 * @param node head node of the list
 * @return 1 if success
 *         0 if error due to memory error
 */
static int init_param_list(Table* table, Node* node) {
    if (!node) return 1;
    
    ValueType type;
    if (!strcmp(node->val.ident, "int")) {
        type = NUMERIC;
    } else {
        type = CHAR;
    }
    if (!insert_entry(table, init_entry(type, node->firstChild))) {
        return 0;
    }
    init_param_list(table, node->nextSibling);
    return 1;
}

/**
 * @brief Create a structure for intels about functions
 * 
 * @param node return type of function
 * @return created function
 */
static Function init_function(Node* node) {
    Function fun;
    fun.is_used = false;
    fun.decl_line = node->lineno;
    assing_rtype(&fun, node);
    strcpy(fun.name, node->nextSibling->val.ident);
    init_table(&fun.parameters);
    init_table(&fun.locals);
    if (node->nextSibling->nextSibling->label == ListTypVar) {
        init_param_list(&fun.parameters, node->nextSibling->nextSibling->firstChild);
    }
    return fun;
}

/**
 * @brief Allocate more memory for a collection
 * 
 * @param collection collection to re alloc
 * @return 1 if success
 *         0 if fail due to memory error 
 */
static int realloc_collection(FunctionCollection* collection) {
    int next_len = collection->max_len + DEFAULT_LENGTH;

    Function* temp = (Function*)realloc(collection->funcs, sizeof(Function)*next_len);
    if (!temp) {
        add_error(EXCEPTION, MEMORY_ERROR, "reallocation failed");
        return 0;
    }
    collection->funcs = temp;
    collection->max_len = next_len;
    return 1;
}

/**
 * @brief Insert function in collection
 * 
 * @param collection collection to insert
 * @param fun function to insert
 * @return 1 if success
 *         0 if error due to memory error
 */
static int insert_function(FunctionCollection* collection, Function fun) {
    if (!collection) return 0;
    if (is_in_collection(collection, fun.name) != -1) {
        add_line_error(EXCEPTION, ALREADY_DECLARE, fun.decl_line, fun.name);
        return 1; // return 1 to continue
    }

    if (collection->cur_len == collection->max_len) {
        if (!realloc_collection(collection)) return 0;
    }

    collection->funcs[collection->cur_len] = fun;
    collection->cur_len++;
    return 1;
}

/**
 * @brief Initialise a variable collection of given type
 * 
 * @param table table to store entries
 * @param type type of the following variables
 * @param node head node
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int decl_var(Table* table, ValueType type, Node* node, Table* parameters) {
    if (!node) {
        return 1;
    }
    Entry entry = init_entry(type, node);

    if (parameters && is_in_table(parameters, entry.name) != -1) {
        add_line_error(EXCEPTION, ALREADY_DECLARE, entry.decl_line, entry.name);
        return 1;
    } 

    if (!insert_entry(table, entry)) {
        return 0;
    }
    return decl_var(table, type, node->nextSibling, parameters);
}

/**
 * @brief Get the type object from its identifiant
 * 
 * @param ident type identifiant
 * @return type
 */
static ValueType get_type(char ident[IDENT_LEN]) {
    if (!strcmp("int", ident)) return NUMERIC;
    return CHAR; 
}

/**
 * @brief Initialise a collection of variables of differents types
 * 
 * @param table table to store the variables
 * @param node head node
 * @return 1 if success
 *         0 if fail due to memory error
 */
static int decl_vars(Table* table, Node* node, Table* parameters) {
    if (!node) {
        return 1;
    }
    ValueType type = get_type(node->val.ident);
    if (!decl_var(table, type, node->firstChild, parameters)) {
        return 0;
    }
    return decl_vars(table, node->nextSibling, parameters);
}

int init_table(Table* table) {
    if (!table) return 0;

    table->total_size = 0;
    table->cur_len = 0;
    table->array = (Entry*)malloc(sizeof(Entry)*DEFAULT_LENGTH);

    if (!table->array) {
        add_error(EXCEPTION, MEMORY_ERROR, "allocation failed");
        table->max_len = 0;
        return 0;
    }
    table->max_len = DEFAULT_LENGTH;
    return 1;
}

int is_in_table(Table* table, char ident[IDENT_LEN]) {
    if (!table || !table->cur_len) return -1;

    for (int i = 0; i < table->cur_len; i++) {
        if (!strcmp(table->array[i].name, ident)) {
            return i;
        }
    }
    return -1;
}

int init_function_collection(FunctionCollection* collection) {
    if (!collection) return 0;

    collection->cur_len = 0;
    collection->funcs = (Function*)malloc(sizeof(Function)*DEFAULT_LENGTH);

    if (!collection->funcs) {
        add_error(EXCEPTION, MEMORY_ERROR, "allocation failed");
        collection->max_len = 0;
        return 0;
    }
    collection->max_len = DEFAULT_LENGTH;
    return 1;
}

int is_in_collection(FunctionCollection* collection, char ident[IDENT_LEN]) {
    if (!collection || !collection->cur_len) return -1;

    for (int i = 0; i < collection->cur_len; i++) {
        if (!strcmp(collection->funcs[i].name, ident)) {
            return i;
        }
    }
    return -1;
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
    } else if (node->label == DeclFonct) {
        if (!insert_function(collection, init_function(node->firstChild->firstChild))) {
            return 0;
        }
    }
    if (!create_tables(globals, collection, node->firstChild)) {
        return 0;
    }
    return create_tables(globals, collection, node->nextSibling);
}

void print_table(Table table) {
    for (int i = 0; i < table.cur_len; i++) {
        printf("type: %4s | decl_line: %3d | size: %5d | name: %s\n",
               table.array[i].type == NUMERIC ? "int": "char",
               table.array[i].decl_line, 
               table.array[i].size,
               table.array[i].name);
    }
}

void print_collection(FunctionCollection collection) {
    for (int i = 0; i < collection.cur_len; i++) {
        RType type = collection.funcs[i].r_type;
        putchar('\n'); 
        printf("%s %s() - Parameters:\n",
                type == R_INT ? "int" : (type == R_CHAR ? "char": "void"),
                collection.funcs[i].name);
        print_table(collection.funcs[i].parameters);

        printf("%s %s() - Locals:\n",
                type == R_INT ? "int" : (type == R_CHAR ? "char": "void"),
                collection.funcs[i].name);
        print_table(collection.funcs[i].locals);
    }
}
