#include "sematic.h"

#include <stdbool.h>
#include <stdlib.h>

#include "table.h"
#include "error.h"

static const char* type_convert[] = {
    [T_CHAR] = "char",
    [T_INT] = "int",
    [T_VOID] = "void"
};

/**
 * @brief Sort a table of entries using the lexicographic order
 * 
 * @param table 
 */
static void sort_table(Table* table);

/**
 * @brief Sort both local and parameter tables using the lexicographic order
 * 
 * @param collection 
 */
static void sort_collection(FunctionCollection* collection);

/**
 * @brief Sort tables of globals and functions
 * 
 * @param globals 
 * @param collection 
 */
static void sort_tables(Table* globals, FunctionCollection* collection);

/**
 * @brief Check if the main function is correct, according to its parameters
 *        and its return type
 * 
 * @param collection collection of function
 */
static void check_main(const FunctionCollection* collection);

/**
 * @brief Search for declared but non-used symbols in a sigle table
 * 
 * @param table table to search the symbol
 * @param source symbol's identifier
 */
static void search_unused_symbol_table(const Table* table, const char* source);

/**
 * @brief Search for declared but non-used symbols in the global's tables and
 *        for functions
 * 
 * @param globals global's table 
 * @param collection collection of functions
 */
static void search_unused_symbols(const Table* globals,
                                  const FunctionCollection* collection);

/**
 * @brief Check if types are valid when assigning a value to a LValue
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function where the assignation is done
 * @param tree head node of the assignation (with the 'Assignation' label)
 */
static void check_assignation_types(const Table* globals,
                                    const FunctionCollection* collection,
                                    const Function* fun, Node* tree);

/**
 * @brief Check if the return type is the correct, according to function
 *        declaration
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current functin where the return is done
 * @param tree head node of the return (with the 'Return' label)
 */
static void check_return_type(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree);

/**
 * @brief Get the type of an identifier. For functions, it is their return type
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function 
 * @param ident identifier to look for
 * @return
 */
static Types ident_type(const Table* globals, const FunctionCollection* collection,
                        const Function* fun, const char* ident);

/**
 * @brief Check if instructions are correcly typed
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function
 * @param tree head node of the instruction
 */
static void check_instruction(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree);

/**
 * @brief Main function for checking types
 * 
 * @param globals global's tables
 * @param collection collection of functions
 * @param tree head node of the programm (the 'Prog' label)
 */
static void check_types(const Table* globals, const FunctionCollection* collection, Node* tree);

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


static void check_main(const FunctionCollection* collection) {
    Function* start_fun = get_function(collection, "main");
    if (!start_fun) {
        error(ERROR, MAIN_MISSING, "no start function found");
        return;
    }
    if (start_fun->r_type != T_INT) {
        wrong_rtype_error(WARNING, "main", type_convert[start_fun->r_type],
                          type_convert[T_INT], start_fun->decl_line,
                          start_fun->decl_col);
        return;
    }
    if (start_fun->parameters.cur_len) {
        error(ERROR, WRONG_PARAMETERS, "main must take no parameters");
    }
}

static void search_unused_symbol_table(const Table* table, const char* source) {
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

static void search_unused_symbols(const Table* globals, const FunctionCollection* collection) {
    // TODO: if symbol is one of the builtin functions, continue
    // (because we prevent redefinition of builtin functions...)
    search_unused_symbol_table(globals, NULL);
    for (int i = 0; i < collection->cur_len; i++) {
        Function fun = collection->funcs[i];
        search_unused_symbol_table(&fun.parameters, fun.name);
        search_unused_symbol_table(&fun.locals, fun.name);
    }
}

static void check_assignation_types(const Table* globals, const FunctionCollection* collection,
                                    const Function* fun, Node* tree) {
    // TODO: Assignation on arrays
    // TODO: check when value is '+55' or '-1'
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

static void check_return_type(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree) {
    Types child_type;
    if (tree->firstChild) {
        check_instruction(globals, collection, fun, tree->firstChild);
        child_type = tree->firstChild->type;
    } else {
        child_type = T_VOID;
    }

    if (fun->r_type != child_type) {
        if (child_type == T_VOID) {
            wrong_rtype_error(ERROR, fun->name, type_convert[child_type],
                            type_convert[fun->r_type], tree->lineno,
                            tree->colno);
        } else if (fun->r_type == T_CHAR && child_type == T_INT) {
            wrong_rtype_error(WARNING, fun->name, type_convert[child_type],
                            type_convert[fun->r_type], tree->lineno,
                            tree->colno);
        }
    }
}

static Types ident_type(const Table* globals, const FunctionCollection* collection,
                        const Function* fun, const char* ident) {
    Entry* entry = find_entry(globals, fun, ident);
    if (entry) {
        return entry->type;
    }
    return get_function(collection, ident)->r_type;
}

// tree is the first instruction of the function
static void check_instruction(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree) {
    if (!tree) return;
    switch (tree->label) {
        case Assignation: check_assignation_types(globals, collection, fun, tree); break;
        case Character: tree->type = T_CHAR; break;
        case Num: tree->type = T_INT; break;
        case Ident: tree->type = ident_type(globals, collection, fun, tree->val.ident); break;
        case Return: check_return_type(globals, collection, fun, tree); return; // return here so we dont parse code after the return
        default: break;
    }
    check_instruction(globals, collection, fun, tree->nextSibling);
}

static void check_types(const Table* globals, const FunctionCollection* collection, Node* tree) {
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
    if (fatal_error()) {
        return 0;
    }
    // checking types requires to have all symbols known so no fatal errors
    check_types(globals, collection, tree);
    return 0;
}
