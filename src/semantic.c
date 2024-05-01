#include "sematic.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "table.h"
#include "errors.h"
#include "types.h"

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
 * @return 0 in case of error else 1 if success
 */
static int check_main(const FunctionCollection* collection);

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
 * @return 0 in case of error else 1 if success
 */
static int check_assignation_types(const Table* globals,
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
 *             the 'type' attribute of the node is set according to the function
 *             prototype or raise an exception
 * @return 0 in case of error else 1 if success
 */
static int check_return_type(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, Node* tree);

/**
 * @brief Check if the parameters to a function are correct, in terms of 
 *        order, number and type
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function where the other function call is
 * @param called function to be called
 * @param tree head node of parameters, either with the 'NoParam' label or 
 *             'ListExp' for a list of parameters
 * @return 0 in case of error else 1 if success
 */
static int check_parameters(const Table* globals, const FunctionCollection* collection,
                            const Function* fun, const Function* called, Node* tree);

/**
 * @brief Check if an entry (a variable) is correctly used
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function where this entry is used
 * @param tree node with the 'Ident' label
 * @param entry used entry
 * @return 0 in case of error else 1 if success
 */
static int check_entry_use(const Table* globals, const FunctionCollection* collection,
                           const Function* fun, Node* tree, const Entry* entry);

/**
 * @brief Check if a function is correctly used
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function where the function is called
 * @param tree node with the 'Ident' label
 * @param function function to be called
 * @return 0 in case of error else 1 if success
 */
static int check_function_use(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree, const Function* function);

/**
 * @brief Get the type of an identifier and if it is correctly used.
 *        This also applied for function
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function 
 * @param ident identifier to look for
 * @param tree node with the 'Ident' label
 * @return 0 in case of error else 1 if success
 */
static int ident_type(const Table* globals, const FunctionCollection* collection,
                      const Function* fun, Node* tree);

/**
 * @brief Check the user correctly perform arithmetics. This verification is
 *        type-based
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function where the arithmetic is performed
 * @param tree head node of the arthmetic. Its label must be one of the
 *             following: 'Eq', 'Order, 'Or', 'And', 'Negation', 'DivStar'
 *             or 'AddSub'
 * @return 0 in case of error else 1 if success
 */
static int check_arithm_type(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, Node* tree);

/**
 * @brief Check types for condition
 * 
 * @param globals global's table
 * @param collection collection of function
 * @param fun current function where the condition is evaludated
 * @param tree head node with the 'If' or 'While' label
 * @return 0 in case of error else 1 if success
 */
static int check_cond_type(const Table* globals, const FunctionCollection* collection,
                           const Function* fun, Node* tree);

/**
 * @brief Check if instructions are correcly typed
 * 
 * @param globals global's table
 * @param collection collection of functions
 * @param fun current function
 * @param tree head node of the instruction
 */
static int check_instruction(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, Node* tree);

/**
 * @brief Main function for checking types
 * 
 * @param globals global's tables
 * @param collection collection of functions
 * @param tree head node of the programm (the 'Prog' label)
 */
static int check_types(const Table* globals, const FunctionCollection* collection,
                       Node* tree);

static void sort_table(Table* table) {
    qsort(table->array, table->cur_len, sizeof(Entry), compare_entries);
    table->sorted = true;
}

static void sort_collection(FunctionCollection* collection) {
    qsort(collection->funcs, collection->cur_len, sizeof(Function),
          compare_functions);
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

static int check_main(const FunctionCollection* collection) {
    Function* start_fun = get_function(collection, "main");
    if (!start_fun) {
        error(ERROR, "no start function found");
        return 0;
    }
    if (start_fun->r_type != T_INT) {
        wrong_rtype_error(ERROR, "main", start_fun->r_type, T_INT,
                          start_fun->decl_line, start_fun->decl_col);
        return 0;
    }
    if (start_fun->parameters.cur_len) {
        error(ERROR, "main must take no parameters");
        return 0;
    }
    return 1;
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

static void search_unused_symbols(const Table* globals,
                                  const FunctionCollection* collection) {
    search_unused_symbol_table(globals, NULL);
    for (int i = 0; i < collection->cur_len; i++) {
        Function fun = collection->funcs[i];
        search_unused_symbol_table(&fun.parameters, fun.name);
        search_unused_symbol_table(&fun.locals, fun.name);
    }
}

static int check_assignation_types(const Table* globals, const FunctionCollection* collection,
                                    const Function* fun, Node* tree) {
    if (!check_instruction(globals, collection, fun, FIRSTCHILD(tree))) return 0;
    if (!check_instruction(globals, collection, fun, SECONDCHILD(tree))) return 0;

    t_type t_dest = FIRSTCHILD(tree)->type;
    t_type t_value = SECONDCHILD(tree)->type;

    // if we tried to assign an int to a char, display a warning
    if (t_dest == T_CHAR && t_value == T_INT) {
        assignation_error(WARNING,
                          FIRSTCHILD(tree)->val.ident,
                          FIRSTCHILD(tree)->type,
                          SECONDCHILD(tree)->type,
                          tree->lineno, tree->colno);
        return 1; // when its a warning we continue
    } else if (t_dest != t_value) {                     // two types are different
        if ((t_dest != T_INT && t_value != T_CHAR)      // not a cast char to int
            || is_array(t_dest)                         // if one of them is either an array 
            || is_array(t_value)                        //
            || t_value == T_VOID) {                     // if we tried to assign a void value
            assignation_error(ERROR,
                              FIRSTCHILD(tree)->val.ident,
                              FIRSTCHILD(tree)->type,
                              SECONDCHILD(tree)->type,
                              tree->lineno, tree->colno);
            return 0;
        }
    }
    return 1;
}

static int check_return_type(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree) {
    t_type child_type;
    
    // if the user is returning a value 
    if (FIRSTCHILD(tree)) {

        // forbid to return when the return type of the function is void
        if (fun->r_type == T_VOID) {
            line_error(ERROR, "void expression not allowed in return", tree->lineno, tree->colno);
            return 0;
        }

        // check return expression
        if (!check_instruction(globals, collection, fun, FIRSTCHILD(tree))) return 0;
        
        // assigning the return type
        child_type = FIRSTCHILD(tree)->type;
    } else {
        // If nothing is return then is void by default
        child_type = T_VOID;
    }

    // if the return type of the function and the return value are different
    if (fun->r_type != child_type) {
        // check if it is a cast from an int to char
        if (fun->r_type == T_CHAR && child_type == T_INT) {
            wrong_rtype_error(WARNING, fun->name, child_type, fun->r_type,
                              tree->lineno, tree->colno);
            return 1; // when its a warning we continue
        } else if (!(fun->r_type == T_INT && child_type == T_CHAR)) {
            // if it's not a cast from a char to an int (which is valid)
            wrong_rtype_error(ERROR, fun->name, child_type, fun->r_type,
                              tree->lineno, tree->colno);
            return 0;
        }
    }
    return 1;
}

static int check_parameters(const Table* globals, const FunctionCollection* collection,
                             const Function* fun, const Function* called, Node* tree) {
    Node* head = tree;
    Entry entry;
    int i;
    
    // looping over each given parameters and the expected ones
    for (i = 0; head != NULL && i < called->parameters.cur_len; i++) {
        
        // if there is an error while checking sub-expression
        if (!check_instruction(globals, collection, fun, head)) return 0;
        
        entry = called->parameters.array[i];

        // one of them is an array
        if (is_array(head->type) || is_array(entry.type)) {
            // if one of them is not an array
            if (!(is_array(entry.type) && is_array(head->type))) {
                invalid_parameter_type(ERROR, called->name, entry.name,
                                       entry.type, head->type, head->lineno,
                                       head->colno);
                return 0;
            }
            // both are arrays but from different types
            if ((is_int(head->type) && is_char(entry.type))
                || (is_char(head->type) && is_int(entry.type))) {
                invalid_parameter_type(ERROR, called->name, entry.name,
                                       entry.type, head->type, head->lineno,
                                       head->colno);
                return 0;
            }
        } else {
            // tw different types
            if (head->type != entry.type) {
                // implicit cast
                if (entry.type == T_INT && head->type == T_CHAR) {
                    head = head->nextSibling;
                    continue;
                }
                ErrorType err_type = ERROR;
                
                // warning cast from int to char
                if (entry.type == T_CHAR && head->type == T_INT) {
                    err_type = WARNING;
                }
                invalid_parameter_type(err_type, called->name, entry.name,
                                       entry.type, head->type, head->lineno,
                                       head->colno);
                return err_type == WARNING;
            }
        }
        head = head->nextSibling;
    }
    // if there is not enough or too much given parameters
    if (head != NULL || i != called->parameters.cur_len) {
        incorrect_function_call(called->name, tree->lineno, tree->colno);
        return 0;
    }
    return 1;
}

static int check_entry_use(const Table* globals, const FunctionCollection* collection,
                           const Function* fun, Node* tree, const Entry* entry) {
    if (FIRSTCHILD(tree)) {
        // either entry is an array and user tries to access it
        // or the user think its a function which entry is not

        // eliminate cases where is a function
        if (FIRSTCHILD(tree)->label == NoParametres || FIRSTCHILD(tree)->label == ListExp) {
            incorrect_symbol_use(entry->name, entry->type,
                                 T_FUNCTION, tree->lineno, tree->colno);
            return 0;
        }

        // if its an array
        if (is_array(entry->type)) {
            // check if the sub-expression is an integer to access the array
            if (!check_instruction(globals, collection, fun, FIRSTCHILD(tree))) return 0;

            if (FIRSTCHILD(tree)->type != T_INT && FIRSTCHILD(tree)->type != T_CHAR) {
                incorrect_array_access(entry->name, FIRSTCHILD(tree)->type,
                                       tree->lineno, tree->colno);
                return 0;
            }
            // else the access is valid and the node type is the array type
            tree->type = T_INT;
            if (is_char(entry->type)) {
                tree->type = T_CHAR;
            }
            return 1;
        } else {
            // an non-array entry should not be used as so
            incorrect_symbol_use(entry->name, entry->type, T_ARRAY,
                                 tree->lineno, tree->colno);
        }
    }
    // if its not a function the user tried to call or an array he tried to access
    // its the variable itself
    tree->type = entry->type;
    return 1;
}

static int check_function_use(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree, const Function* function) {
    // check first if we tried to call the function
    if (!FIRSTCHILD(tree)) { // function's name is used as a variable
        incorrect_symbol_use(function->name, T_FUNCTION, T_ARRAY, tree->lineno,
                             tree->colno);
        return 0;
    }

    // else the user tries to call it
    // check if there are parameters
    if (FIRSTCHILD(tree)->label == NoParametres) {  // if no parameters are given
        if (function->parameters.cur_len) {         // if the function requires parameters 
            incorrect_function_call(function->name, tree->lineno, tree->colno);
            return 0;
        }
    } else if (FIRSTCHILD(tree)->label == ListExp) {
        if (!check_parameters(globals, collection, fun, function, FIRSTCHILD(FIRSTCHILD(tree))))
            return 0;
    } else {
        // the user tries to access the function as an array
        incorrect_symbol_use(function->name, T_FUNCTION,
                             T_ARRAY, tree->lineno, tree->colno);
        return 0;
    }
    // the function can be called
    // the type value of the node is the return value of the function
    tree->type = function->r_type;
    return 1;
}

static int ident_type(const Table* globals, const FunctionCollection* collection,
                      const Function* fun, Node* tree) {
    Entry* entry = find_entry(globals, fun, tree->val.ident);
    if (entry) {
        return check_entry_use(globals, collection, fun, tree, entry);
    }
    Function* function = get_function(collection, tree->val.ident);
    return check_function_use(globals, collection, fun, tree, function);
}

static int check_arithm_type(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree) {
    if (!check_instruction(globals, collection, fun, FIRSTCHILD(tree))) return 0;
    if (!check_instruction(globals, collection, fun, SECONDCHILD(tree))) return 0;
    
    t_type ltype = FIRSTCHILD(tree)->type;
    if (tree->label == AddSub) {
        // no second child = unary plus or minus
        if (!(SECONDCHILD(tree))) {
            // invalid type for unary opertation
            if (ltype != T_INT && ltype != T_CHAR) {
                invalid_operation(tree->val.ident, ltype,
                                  tree->lineno, tree->colno);
                return 0;
            }
            tree->type = ltype;
            return 1;
        }
    } else if (tree->label == Negation) {
        if (ltype != T_INT && ltype != T_CHAR) {
                invalid_operation(tree->val.ident, ltype,
                                  tree->lineno, tree->colno);
                return 0;
            }
            tree->type = ltype;
            return 1;
    }
    t_type rtype = SECONDCHILD(tree)->type;
    if (ltype != T_INT && ltype != T_CHAR) {
        invalid_operation(tree->val.ident, ltype,
                            tree->lineno, tree->colno);
        return 0;
    } else if (rtype != T_INT && rtype != T_CHAR) {
        invalid_operation(tree->val.ident, rtype,
                            tree->lineno, tree->colno);
        return 0;
    }
    tree->type = T_INT; // all operations are cast to integer
    return 1;
}

static int check_cond_type(const Table* globals, const FunctionCollection* collection,
                            const Function* fun, Node* tree) {
    if (!check_instruction(globals, collection, fun, FIRSTCHILD(tree))) return 0;
    if (FIRSTCHILD(tree)->type != T_INT && FIRSTCHILD(tree)->type != T_CHAR) {
        invalid_condition(FIRSTCHILD(tree)->type, tree->lineno,
                          tree->colno);
        return 0;
    }
    return 1;
}

// tree is the first instruction of the function
static int check_instruction(const Table* globals, const FunctionCollection* collection,
                              const Function* fun, Node* tree) {
    if (!tree) return 1;    
    switch (tree->label) {
        case Assignation: return check_assignation_types(globals, collection, fun, tree);
        case Character: tree->type = set_type(tree->type, T_CHAR); return 1;
        case Num: tree->type = set_type(tree->type, T_INT); return 1;
        case Ident: return ident_type(globals, collection, fun, tree);
        case Return: return check_return_type(globals, collection, fun, tree);
        case Eq: case Order:
        case Or: case And: case Negation:
        case DivStar: case AddSub: return check_arithm_type(globals, collection, fun, tree);
        case If: case While: return check_cond_type(globals, collection, fun, tree);
        default: return 1;
    }
}

static int check_types(const Table* globals, const FunctionCollection* collection, Node* tree) {
    Node* decl_fonct_node = FIRSTCHILD(SECONDCHILD(tree)), *head_instr;
    Function* fun;

    for (; decl_fonct_node;) {
        fun = get_function(collection,
            SECONDCHILD(FIRSTCHILD(decl_fonct_node))->val.ident);

        head_instr = FIRSTCHILD(SECONDCHILD(SECONDCHILD(decl_fonct_node)));

        for (; head_instr;) {
            if (!check_instruction(globals, collection, fun, head_instr)) {
                return 0;
            }
            head_instr = head_instr->nextSibling;
        }
        decl_fonct_node = decl_fonct_node->nextSibling;
    }
    return 1;
}

int check_sem(Table* globals, FunctionCollection* collection, Node* tree) {
    sort_tables(globals, collection);
    if (!check_main(collection)) return 0;
    
    // no need to check this one, its only for "notes"
    search_unused_symbols(globals, collection); 

    return check_types(globals, collection, tree);
}
