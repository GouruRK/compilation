#include "errors.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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

void wrong_rtype_error(ErrorType type, const char* symbol, const char* current_type,
                       const char* expected_type, int line, int col) {
    Error err = (Error){.type = type,
                        .line = line,
                        .col  = col,
                        .has_line = true};
    snprintf(err.message, ERROR_LEN,
             "'%s' return type must be '%s' instead of '%s'",
             symbol, expected_type, current_type);
    
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

void assignation_error(ErrorType type, const char* symbol, const char* dest_type,
                       const char* source_type, int line, int col) {
    Error err = (Error){.type = type,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "trying to assign to '%s' of type '%s' a value of type '%s'",
             symbol, dest_type, source_type);
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

void incorrect_array_access(const char* name, const char* access_type, int line,
                            int col) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "trying to access array '%s' with an expression of type '%s'",
             name, access_type);
    print_error(&err);
}

void invalid_operation(const char* operation, const char* type, int line,
                       int col) {
    Error err = (Error){.type = ERROR,
                        .line = line,
                        .col = col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "invalid operation '%s' on type '%s'",
             operation, type);
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
