#ifndef TABLE_H
#define TABLE_H

#include <stdbool.h>
#include "tree.h"

#define DEFAULT_LENGTH 10

typedef enum { // type retours
    R_INT, R_CHAR, R_VOID
} RType;

typedef enum { // size of types in byte
    S_INT = 4,
    S_CHAR = 1
} TypeSize;

typedef struct {            // variable
    bool array;             // if variable is an array
    bool is_used;           // if used
    int decl_line;          // declaration line
    int size;               // size in bytes
    ValueType type;         // type of variable
    char name[IDENT_LEN];   // variable name
} Entry;

typedef struct {            // symbol table
    int cur_len;            // curent length of the table
    int max_len;            // maximum length of the table
    int total_size;         // total size of stored variables in bytes
    Entry* array;           // entry array
} Table;

typedef struct {            // symbol table for functions
    bool is_used;           // if the function is used
    int decl_line;          // declaration line
    RType r_type;           // returned type
    char name[IDENT_LEN];   // function name
    Table table;            // parameters and locals variables
} Function;

typedef struct {            // array of symbol table for functions
    int cur_len;            // current number of stored functions
    int max_len;            // maximum length 
    Function* funcs;        // functions array
} FunctionCollection;


Entry init_entry(ValueType type, Node* node);
int init_table(Table* table);
int is_in_table(Table* table, char ident[IDENT_LEN]);
int insert_entry(Table* table, Entry entry);
Function init_function(Node* node);
int init_function_collection(FunctionCollection* collection);
int is_in_collection(FunctionCollection* collection, char ident[IDENT_LEN]);
int insert_function(FunctionCollection* collection, Function fun);
void free_table(Table* table);
void free_collection(FunctionCollection* collection);

#endif
