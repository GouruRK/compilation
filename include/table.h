#ifndef TABLE_H
#define TABLE_H

#include <stdbool.h>

#include "tree.h"
#include "types.h"

#define DEFAULT_LENGTH 10

typedef struct {            // variable
    bool is_used;           // if used
    int address;            // address
    int decl_line;          // declaration line
    int decl_col;           // declaration column
    int size;               // size in bytes
    t_type type;            // type of variable
    char name[IDENT_LEN];   // variable name
} Entry;

typedef struct {            // symbol table
    bool sorted;            // if entries has been sorted by name
    int cur_len;            // curent length of the table
    int max_len;            // maximum length of the table
    int total_bytes;        // bytes used in the table
    int offset;             // offset in bytes
    Entry* array;           // entry array
} Table;

typedef struct {            // symbol table for functions
    bool is_used;           // if the function is used
    int decl_line;          // declaration line
    int decl_col;           // declaration column
    t_type r_type;          // returned type
    char name[IDENT_LEN];   // function name
    Table parameters;       // parameters
    Table locals;           // locals
} Function;

typedef struct {            // array of symbol table for functions
    bool sorted;            // if functions has been sorted by name
    int cur_len;            // current number of stored functions
    int max_len;            // maximum length 
    Function* funcs;        // functions array
} FunctionCollection;

/**
 * @brief Function to compare 2 entries based on their names
 * 
 * @param entry1
 * @param entry2 
 * @return int /0 if equal/ 1 < if entry1 < entry2/ 1 > if entry1 > entry2
 */
int compare_entries(const void* entry1, const void* entry2);

int compare_functions(const void* fun1, const void* fun2);
int compare_ident_entry(const void* ident, const void* entry);
int compare_ident_fun(const void* ident, const void* fun);

/**
 * @brief Create a table structure that contains entries
 * 
 * @param table table to create
 * @return 1 if success
 *         0 if failed due to memory error
 */
int init_table(Table* table);

/**
 * @brief Check if an identifiant is in the given table.
 * 
 * @param table table 
 * @param ident identifiant to check
 * @return index of the entry of the given identifiant if it is in the table
 *         else -1
 */
int is_in_table(const Table* table, const char ident[IDENT_LEN]);

Entry* get_entry(const Table* table, const char ident[IDENT_LEN]);

/**
 * @brief Create a collection of functions 
 * 
 * @param collection collection to create
 * @return 1 if success
 *         0 if error due to memory error
 */
int init_function_collection(FunctionCollection* collection);

/**
 * @brief Check if a function identifiant is in the given collection
 * 
 * @param collection collection to search 
 * @param ident identifiant to check
 * @return index of the entry of the given identifiant if it is in the table
 *         else -1
 */
int is_in_collection(const FunctionCollection* collection, const char ident[IDENT_LEN]);

Entry* find_entry(const Table* globals, const Function* fun, const char ident[IDENT_LEN]);

Function* get_function(const FunctionCollection* collection, const char ident[IDENT_LEN]);

/**
 * @brief Free allocated memory for table
 * 
 * @param table table to free
 */
void free_table(Table* table);

/**
 * @brief Free allocated memory for functions
 *        Free also their tables
 * 
 * @param collection collection to free
 */
void free_collection(FunctionCollection* collection);

/**
 * @brief Create a symbols table for the given tree
 * 
 * @param globals table for globals variables
 * @param collection collection of functions
 * @param node root
 * @return 1 if success
 *         0 if fail due to memory error
 */
int create_tables(Table* globals, FunctionCollection* collection, Node* node);

/**
 * @brief Print symbol table content
 * 
 * @param table table to print
 */
void print_table(Table table);

/**
 * @brief Print function collection content
 * 
 * @param collection collection to print
 */
void print_collection(FunctionCollection collection);

#endif
