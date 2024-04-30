#include "errors.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "types.h"

bool init = false;
char filename[64];

static char* types[] = {
    [WARNING] = PURPLE "warning" RESET,
    [NOTE] = CYAN "note" RESET,
    [ERROR] = RED "error" RESET
};

static int error_count[] = {
    [WARNING] = 0,
    [NOTE] = 0,
    [ERROR] = 0
};

static const char* type_convert[] = {
    [T_NONE]           = "none",
    [T_CHAR]           = "char",
    [T_INT]            = "int",
    [T_VOID]           = "void",
    [T_ARRAY]          = "array",
    [T_ARRAY | T_CHAR] = "array[char]",
    [T_ARRAY | T_INT]  = "array[int]",
    [T_FUNCTION]       = "function",
};

void init_error(const char* source) {
    init = true;
    strcpy(filename, source);
}

static void print_error(Error *error) {
    if (error->has_line) {
        fprintf(stderr, "%s:%d:%d %s: %s\n",
                init ? filename: "", error->line, error->col, 
                types[error->type], error->message);
    } else {
        fprintf(stderr, "%s: %s: %s\n",
                init ? filename: "", types[error->type], error->message);
    }
    error_count[error->type]++;
}

void memory_error(void) {
    Error error = (Error){.type = ERROR,
                          .has_line = false,
                          .message = "error while allocating memory"};
    print_error(&error);
}

void already_declared_error(const char* symbol, int line, int col,
                            int last_line) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col  = col,
                        .has_line = true};
    snprintf(err.message,
             ERROR_LEN,
             "symbol '%s' already declared at line %d",
             symbol, last_line);
    
    print_error(&err);
}

void wrong_rtype_error(ErrorType type, const char* symbol, t_type current_type,
                       t_type expected_type, int line, int col) {
    Error err = (Error){.type = type,
                        .line = line,
                        .col  = col,
                        .has_line = true};
    snprintf(err.message, ERROR_LEN,
             "'%s' return type must be '%s' instead of '%s'",
             symbol, type_convert[expected_type], type_convert[current_type]);
    
    print_error(&err);
}

void use_of_undeclare_symbol(const char* symbol, int line, int col) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "uses of undeclared symbol: '%s'", symbol);
    print_error(&err);
}

void unused_symbol(const char* symbol, int line, int col) {
    Error err = (Error){.type = NOTE,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "unused symbol: '%s'", symbol);
    print_error(&err);
}

void unused_symbol_in_function(const char* function, const char* symbol,
                               int line, int col) {
    Error err = (Error){.type = NOTE,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "unused symbol: '%s' in function '%s'", symbol, function);
    print_error(&err);
}

void assignation_error(ErrorType type, const char* symbol, t_type dest_type,
                       t_type source_type, int line, int col) {
    Error err = (Error){.type = type,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "trying to assign to '%s' of type '%s' a value of type '%s'",
             symbol, type_convert[dest_type], type_convert[source_type]);
    print_error(&err);
}

void redefinition_of_builtin_functions(const char* function, int line,
                                       int col) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "trying to redefine builtin function '%s'", function);
    print_error(&err);
}

void incorrect_array_access(const char* name, t_type access_type, int line,
                            int col) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "trying to access array '%s' with an expression of type '%s'",
             name, type_convert[access_type]);
    print_error(&err);
}

void invalid_operation(const char* operation, t_type type, int line,
                       int col) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "invalid operation '%s' on type '%s'",
             operation, type_convert[type]);
    print_error(&err);
}

void invalid_condition(t_type type, int line, int col) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "invalid type for condition: expected 'int', got '%s'",
             type_convert[type]);
    print_error(&err);
}

void incorrect_function_call(const char* function, int line, int col) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "incorrect function '%s' call", function);
    print_error(&err);
}

void invalid_parameter_type(ErrorType type, const char* function,
                            const char* param_name, t_type expected,
                            t_type current, int line, int col) {
    Error err = (Error){.type = type,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "incorrect parameter '%s' type while trying to call '%s': expected type"
             " '%s', got '%s'", param_name, function, type_convert[expected],
             type_convert[current]);
    print_error(&err);
}

void incorrect_symbol_use(const char* symbol, t_type sym_type,
                          t_type other_type, int line, int col) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "entry %s of type %s is not typed %s",
             symbol, type_convert[sym_type], type_convert[other_type]);
    print_error(&err);
}

void incorrect_array_decl(const char* symbol, int line, int col) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN, "array '%s' size cannot be 0", symbol);
    print_error(&err);
}

void error(ErrorType type, const char* message) {
    Error err = (Error){.type = type, .has_line = false};
    strcpy(err.message, message);
    print_error(&err);
}

bool fatal_error(void) {
    return error_count[ERROR];
}
