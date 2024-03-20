#ifndef TABLE_H
#define TABLE_H

#include <stdbool.h>
#include "tree.h"

typedef enum { // type retours
    R_INT, R_CHAR, R_VOID
} RType;

typedef struct {            // variable
    bool array;             // if variable is an array
    bool is_used;           // if used
    int decl_line;          // declaration line
    int size;               // size in bytes
    ValueType type;         // type of variable
    Value val;              // current value
} Entry;

typedef struct {            // symbols table
    int cur_len;            // curent length of the table
    int max_len;            // maximum length of the table
    int total_size;         // total size of stored variables in bytes
    Entry* array;           // entry array
} Table;

typedef struct {            // symbols table for functions
    bool is_used;           // if the function is used
    int decl_line;          // declaration line
    int adr;                // address
    RType r_type;           // returned type
    char name[IDENT_LEN];  // function name
    Table table;            // parameters and locals variables
} Function;

typedef struct {            // array of symbols table for functions
    int cur_len;            // current number of stored functions
    int max_len;            // maximum length 
    Function* funcs;        // functions array
} FunctionCollection;

#endif
