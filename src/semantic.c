#include "sematic.h"

#include <stdbool.h>
#include <stdlib.h>

#include "table.h"
#include "errors.h"

static const char* type_convert[] = {
    [T_CHAR] = "char",
    [T_INT] = "int",
    [T_VOID] = "void",
    [T_ARRAY] = "array",
    [T_FUNCTION] = "function"
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
 * @brief Check if the parameters to a function are correct, in terms of 
 *        order, number and type
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function where the other function call is
 * @param called function to be called
 * @param tree node of parameters
 */
static void check_parameters(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Function* called, Node* tree);

/**
 * @brief Get the type of an identifier. For functions, it is their return type
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function 
 * @param ident identifier to look for
 * @return
 */
static void ident_type(const Table* globals, const FunctionCollection* collection,
                        const Function* fun, Node* tree);

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
        // dont sort parameters in order to correctly check the variables in the call
    }
}


static void check_main(const FunctionCollection* collection) {
    Function* start_fun = get_function(collection, "main");
    if (!start_fun) {
        error(ERROR, "no start function found");
        return;
    }
    if (start_fun->r_type != T_INT) {
        wrong_rtype_error(WARNING, "main", type_convert[start_fun->r_type],
                          type_convert[T_INT], start_fun->decl_line,
                          start_fun->decl_col);
        return;
    }
    if (start_fun->parameters.cur_len) {
        error(ERROR, "main must take no parameters");
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
    search_unused_symbol_table(globals, NULL);
    for (int i = 0; i < collection->cur_len; i++) {
        Function fun = collection->funcs[i];
        search_unused_symbol_table(&fun.parameters, fun.name);
        search_unused_symbol_table(&fun.locals, fun.name);
    }
}

static void check_assignation_types(const Table* globals, const FunctionCollection* collection,
                                    const Function* fun, Node* tree) {
    check_instruction(globals, collection, fun, FIRSTCHILD(tree));
    check_instruction(globals, collection, fun, SECONDCHILD(tree));
    Types t_dest = FIRSTCHILD(tree)->type;
    Types t_value = SECONDCHILD(tree)->type;
    if (t_dest == T_CHAR && t_value == T_INT) { // warning when casting char to int
        assignation_error(WARNING,
                          FIRSTCHILD(tree)->val.ident,
                          type_convert[tree->firstChild->type],
                          type_convert[SECONDCHILD(tree)->type],
                          tree->lineno, tree->colno);
    } else if (t_dest != t_value) {
        if (t_dest != T_INT && t_value != T_CHAR) {
            assignation_error(ERROR,
                              FIRSTCHILD(tree)->val.ident,
                              type_convert[tree->firstChild->type],
                              type_convert[SECONDCHILD(tree)->type],
                              tree->lineno, tree->colno);
        }
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

static void check_parameters(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Function* called, Node* tree) {
    Node* head = tree;
    Entry entry;
    int i;
    for (i = 0; head != NULL && i < called->parameters.cur_len; i++) {
        check_instruction(globals, collection, fun, head);
        entry = called->parameters.array[i];
        if (head->type != entry.type) {
            if (entry.type == T_INT && head->type == T_CHAR) {
                head = head->nextSibling;
                continue;
            }
            ErrorType type = ERROR;
            if (head->type == T_INT && entry.type == T_CHAR) {
                type = WARNING;
            }
            invalid_parameter_type(type, called->name, entry.name,
                                   type_convert[entry.type],
                                   type_convert[head->type],
                                   head->lineno, head->colno);
        }
        head = head->nextSibling;
    }
    if (head != NULL || i != called->parameters.cur_len) {
        incorrect_function_call(called->name, tree->lineno, tree->colno);
    }
}

static void ident_type(const Table* globals, const FunctionCollection* collection,
                        const Function* fun, Node* tree) {
    Entry* entry = find_entry(globals, fun, tree->val.ident);
    if (entry) { // ident is a variable
        if (entry->array) {
            if (FIRSTCHILD(tree)) {
                check_instruction(globals, collection, fun, FIRSTCHILD(tree));
                if (FIRSTCHILD(tree)->type == T_INT || FIRSTCHILD(tree)->type == T_CHAR) {
                    tree->type = entry->type;
                } else {
                    incorrect_array_access(entry->name,
                                           type_convert[FIRSTCHILD(tree)->type],
                                           tree->lineno,
                                           tree->colno);
                    tree->type = T_INT; // set to int to continue
                }
            } else {
                tree->type = T_ARRAY;
            }
        } else {
            tree->type = entry->type;
        }
    } else { // ident is a function
        Function* function = get_function(collection, tree->val.ident);
        if (FIRSTCHILD(tree)) {
            if (FIRSTCHILD(tree)->label == NoParametres) {
                if (function->parameters.cur_len) {
                    incorrect_function_call(function->name, tree->lineno, tree->colno);
                }
                tree->type = function->r_type;
            } else {
                check_parameters(globals, collection, fun, function, FIRSTCHILD(FIRSTCHILD(tree)));
                tree->type = function->r_type;
            }
        } else {
            tree->type = T_FUNCTION;
        }
    }
}

static void check_arithm_type(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree) {
    check_instruction(globals, collection, fun, FIRSTCHILD(tree));
    check_instruction(globals, collection, fun, SECONDCHILD(tree));
    Types ltype = FIRSTCHILD(tree)->type;
    if (tree->label == AddSub) {
        if (!(SECONDCHILD(tree))) { // unary plus or minus
            if (ltype != T_INT && ltype != T_CHAR) {
                invalid_operation(tree->val.ident, type_convert[ltype],
                                  tree->lineno, tree->colno);
                tree->type = T_INT;
            }
            return;
        }
    }
    Types rtype = SECONDCHILD(tree)->type;
    if (ltype != T_INT && ltype != T_CHAR) {
        invalid_operation(tree->val.ident, type_convert[ltype],
                            tree->lineno, tree->colno);
    } else if (rtype != T_INT && rtype != T_CHAR) {
        invalid_operation(tree->val.ident, type_convert[rtype],
                            tree->lineno, tree->colno);
    }
    tree->type = T_INT;
}

static void check_cond_type(const Table* globals, const FunctionCollection* collection,
                            const Function* fun, Node* tree) {
    check_instruction(globals, collection, fun, FIRSTCHILD(tree));
    if (FIRSTCHILD(tree)->type != T_INT && FIRSTCHILD(tree)->type != T_CHAR) {
        invalid_condition(type_convert[FIRSTCHILD(tree)->type], tree->lineno,
                          tree->colno);
    }
}

// tree is the first instruction of the function
static void check_instruction(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree) {
    if (!tree) return;
    switch (tree->label) {
        case Assignation: check_assignation_types(globals, collection, fun, tree); break;
        case Character: tree->type = T_CHAR; break;
        case Num: tree->type = T_INT; break;
        case Ident: ident_type(globals, collection, fun, tree); break;
        case Return: check_return_type(globals, collection, fun, tree); return; // return here so we dont parse code after the return
        case Eq: case Order:
        case Or: case And: case Negation:
        case DivStar: case AddSub: check_arithm_type(globals, collection, fun, tree); break;
        case If: case While: check_cond_type(globals, collection, fun, tree); break;
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
