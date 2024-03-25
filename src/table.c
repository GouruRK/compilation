#include "table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

// node est le nom de la variable
Entry init_entry(ValueType type, Node* node) {
    Entry entry;
    entry.array = node->array;
    entry.decl_line = node->lineno;
    entry.is_used = false;
    entry.size = compute_size(type, node);
    entry.type = type;
    strcpy(entry.name, node->val.ident);
    return entry;
}

int init_table(Table* table) {
    if (!table) return 0;

    table->total_size = 0;
    table->cur_len = 0;
    table->array = (Entry*)malloc(sizeof(Entry)*DEFAULT_LENGTH);

    if (!table->array) {
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

static int realloc_table(Table* table) {
    int next_len = table->max_len + DEFAULT_LENGTH;

    Entry* temp = realloc(table->array, sizeof(Entry)*next_len);
    if (!temp) {
        return 0;
    }
    table->array = temp;
    table->max_len = next_len;
    return 1;
}

int insert_entry(Table* table, Entry entry) {
    if (!table) return 0;
    if (is_in_table(table, entry.name) != -1) return 0;

    if (table->cur_len == table->max_len) {
        if (!realloc_table(table)) return 0;
    }

    table->array[table->cur_len] = entry;
    table->cur_len++;
    table->total_size += entry.size;
    return 1;
}

static void assing_rtype(Function* fun, Node* node) {
    if (!strcmp(node->val.ident, "int")) {
        fun->r_type = R_INT;
    } else if (!strcmp(node->val.ident, "char")) {
        fun->r_type = R_CHAR;
    } else {
        fun->r_type = R_VOID;
    }
}

static void init_param_list(Table* table, Node* node) {
    if (!node) return;
    
    ValueType type;
    if (!strcmp(node->val.ident, "int")) {
        type = NUMERIC;
    } else {
        type = CHAR;
    }
    insert_entry(table, init_entry(type, node->firstChild));
    init_param_list(table, node->nextSibling);
}

// node pointe sur le type de la fonction
Function init_function(Node* node) {
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

int init_function_collection(FunctionCollection* collection) {
    if (!collection) return 0;

    collection->cur_len = 0;
    collection->funcs = (Function*)malloc(sizeof(Function)*DEFAULT_LENGTH);

    if (!collection->funcs) {
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

static int realloc_collection(FunctionCollection* collection) {
    int next_len = collection->max_len + DEFAULT_LENGTH;

    Function* temp = (Function*)realloc(collection->funcs, sizeof(Function)*next_len);
    if (!temp) {
        return 0;
    }
    collection->funcs = temp;
    collection->max_len = next_len;
    return 1;
}

int insert_function(FunctionCollection* collection, Function fun) {
    if (!collection) return 0;
    if (is_in_collection(collection, fun.name) != -1) return 0;

    if (collection->cur_len == collection->max_len) {
        if (!realloc_collection(collection)) return 0;
    }

    collection->funcs[collection->cur_len] = fun;
    collection->cur_len++;
    return 1;
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

static int decl_var(Table* table, ValueType type, Node* node) {
    if (!node) {
        return 1;
    }
    Entry entry = init_entry(type, node);
    insert_entry(table, entry);
    decl_var(table, type, node->nextSibling);
    return 1;
}

static ValueType get_type(char ident[IDENT_LEN]) {
    if (!strcmp("int", ident)) return NUMERIC;
    return CHAR; 
}

static int decl_vars(Table* table, Node* node) {
    if (!node) {
        return 1;
    }
    ValueType type = get_type(node->val.ident);
    decl_var(table, type, node->firstChild);
    decl_vars(table, node->nextSibling);
    return 0;
}

int create_tables(Table* globals, FunctionCollection* collection, Node* node) {
    static bool is_globals_done = false;

    if (!node) {
        return 1;
    }

    if (node->label == DeclVars) {
        if (!is_globals_done) {
            decl_vars(globals, node->firstChild);
            is_globals_done = true;
        } else {
            decl_vars(&(collection->funcs[collection->cur_len - 1].locals), node->firstChild);
        }
    } else if (node->label == DeclFonct) {
        insert_function(collection, init_function(node->firstChild->firstChild));
    }
    create_tables(globals, collection, node->nextSibling);
    create_tables(globals, collection, node->firstChild);
    return 1;
}

void print_table(Table table) {
    for (int i = 0; i < table.cur_len; i++) {
        printf("type: %s\tdecl_line: %3d\tsize: %5d\tname: %s\n",
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
        printf("%s\t%s - Parameters\n",
                type == R_INT ? "int" : (type == R_CHAR ? "char": "void"),
                collection.funcs[i].name);
        print_table(collection.funcs[i].parameters);

        printf("%s\t%s - Locals\n",
                type == R_INT ? "int" : (type == R_CHAR ? "char": "void"),
                collection.funcs[i].name);
        print_table(collection.funcs[i].locals);
    }
}
