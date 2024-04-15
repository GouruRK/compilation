#include "sematic.h"

#include <stdbool.h>
#include <stdlib.h>

#include "table.h"
#include "error.h"

char* type_convert[] = {
    [T_CHAR] = "char",
    [T_INT] = "int",
    [T_VOID] = "void"
};

static void sort_table(Table* table);
static void sort_collection(FunctionCollection* collection);
static void sort_tables(Table* globals, FunctionCollection* collection);
static void check_main(FunctionCollection* collection);
static void search_unused_symbol_table(Table* table, char* source);
static void search_unused_symbols(Table* globals, FunctionCollection* collection);
static void check_assignation_types(Table* globals, FunctionCollection* collection,
                                    Function* fun, Node* tree);
static void check_instruction(Table* globals, FunctionCollection* collection,
                              Function* fun, Node* tree);
static void check_types(Table* globals, FunctionCollection* collection, Node* tree);


static void sort_table(Table* table) {
    qsort(table->array, table->cur_len, sizeof(Entry), compare_entries);
    table->sorted = true;
}

static void sort_collection(FunctionCollection* collection) {
    qsort(collection->funcs, collection->cur_len, sizeof(Function), compare_functions);
    collection->sorted = true;
}

static void sort_tables(Table* globals, FunctionCollection* collection) {
    sort_table(globals);
    sort_collection(collection);
    for (int i = 0; i < collection->cur_len; i++) {
        sort_table(&(collection->funcs[i].locals));
        sort_table(&(collection->funcs[i].parameters));
    }
}


static void check_main(FunctionCollection* collection) {
    Function* start_fun = get_function(collection, "main");
    if (!start_fun) {
        error(ERROR, MAIN_MISSING, "no start function found");
        return;
    }
    if (start_fun->r_type != T_INT) {
        wrong_rtype_error("main", type_convert[start_fun->r_type],
                              type_convert[T_INT], start_fun->decl_line,
                              start_fun->decl_col);
        return;
    }
    if (start_fun->parameters.cur_len) {
        error(ERROR, WRONG_PARAMETERS, "main must take no parameters");
    }
}

static void search_unused_symbol_table(Table* table, char* source) {
    for (int i = 0; i < table->cur_len; i++) {
        Entry entry = table->array[i];
        if (!entry.is_used) {
            if (source) {
                unused_symbol_in_function(source, entry.name, entry.decl_line,
                                          entry.decl_col);    
            } else {
                unused_symbol(entry.name, entry.decl_line, entry.decl_col);
            }
        }
    }
}

static void search_unused_symbols(Table* globals, FunctionCollection* collection) {
    search_unused_symbol_table(globals, NULL);
    for (int i = 0; i < collection->cur_len; i++) {
        Function fun = collection->funcs[i];
        search_unused_symbol_table(&fun.parameters, fun.name);
        search_unused_symbol_table(&fun.locals, fun.name);
    }
}

static void check_assignation_types(Table* globals, FunctionCollection* collection,
                                    Function* fun, Node* tree) {
    // TODO: Assignation sur tableaux
    check_instruction(globals, collection, fun, tree->firstChild);
    check_instruction(globals, collection, fun, SECONDCHILD(tree));
    Types t_dest = tree->firstChild->type;
    Types t_value = SECONDCHILD(tree)->type;
    if (t_dest == T_CHAR && t_dest != t_value) {
        assignation_error(FIRSTCHILD(tree)->val.ident,
                          type_convert[tree->firstChild->type],
                          type_convert[SECONDCHILD(tree)->type],
                          tree->lineno, tree->colno);
    } else if (t_dest == T_INT && t_value == T_VOID) {
        assignation_error(FIRSTCHILD(tree)->val.ident,
                          type_convert[tree->firstChild->type],
                          type_convert[SECONDCHILD(tree)->type],
                          tree->lineno, tree->colno);
    }
}

static Types ident_type(Table* globals, FunctionCollection* collection,
                        Function* fun, Node* tree) {
    Entry* entry = find_entry(globals, fun, tree->val.ident);
    if (entry) {
        return entry->type;
    }
    return get_function(collection, tree->val.ident)->r_type;
}

// tree is the first instruction of the function
static void check_instruction(Table* globals, FunctionCollection* collection,
                              Function* fun, Node* tree) {
    if (!tree) return;
    switch (tree->label) {
        case Assignation: check_assignation_types(globals, collection, fun, tree); break;
        case Character: tree->type = T_CHAR; break;
        case Num: tree->type = T_INT; break;
        case Ident: tree->type = ident_type(globals, collection, fun, tree);
        default: break;
    }
    check_instruction(globals, collection, fun, tree->nextSibling);
}

static void check_types(Table* globals, FunctionCollection* collection, Node* tree) {
    Node* decl_fonct_node = FIRSTCHILD(SECONDCHILD(tree));
    Function* fun;

    for (; decl_fonct_node != NULL;) {
        fun = get_function(collection,
            decl_fonct_node->firstChild->firstChild->nextSibling->val.ident);
        check_instruction(globals, collection, fun, FIRSTCHILD(SECONDCHILD(SECONDCHILD(decl_fonct_node))));
        decl_fonct_node = decl_fonct_node->nextSibling;
    }
}

int check_sem(Table* globals, FunctionCollection* collection, Node* tree) {
    sort_tables(globals, collection);
    check_main(collection);
    search_unused_symbols(globals, collection);
    check_types(globals, collection, tree);
    return 0;
}
