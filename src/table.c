#include "table.h"

#include <stdlib.h>
#include <string.h>

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
    init_table(&fun.table);
    init_param_list(&fun.table, node->nextSibling->nextSibling);
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
        free(collection->funcs[i].table.array);
    }
    free(collection->funcs);
}
