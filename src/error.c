#include "error.h"

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

void init_error(char* source) {
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
}

void memory_error(void) {
    Error error = (Error){.type = ERROR,
                          .type = MEMORY_ERROR,
                          .has_line = false,
                          .message = "error while allocating memory"};
    error_count[error.type]++;
    print_error(&error);
}

void already_declared_error(char* symbol, int decl_line, int decl_col,
                                int last_decl_line) {
    Error err = (Error){.type = ERROR,
                        .code = ALREADY_DECLARE,
                        .line = decl_line,
                        .col  = decl_col,
                        .has_line = true};
    snprintf(err.message,
             ERROR_LEN,
             "symbol '%s' already declared at line %d",
             symbol, last_decl_line);
    
    error_count[err.type]++;
    print_error(&err);
}

void wrong_rtype_error(char* symbol, char* current_type,
                           char* expected_type, int decl_line, int decl_col) {
    Error err = (Error){.type = WARNING,
                        .code = WRONG_RTYPE,
                        .line = decl_line,
                        .col  = decl_col,
                        .has_line = true};
    snprintf(err.message, ERROR_LEN,
             "'%s' return type must be '%s' instead of '%s'",
             symbol, expected_type, current_type);
    
    error_count[err.type]++;
    print_error(&err);
}

void use_of_undeclare_symbol(char* symbol, int decl_line, int decl_col) {
    Error err = (Error){.type = ERROR,
                        .code = USE_OF_UNDECLARE_SYMBOL,
                        .line = decl_line,
                        .col = decl_col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "uses of undeclared symbol: '%s'", symbol);
    error_count[err.type]++;
    print_error(&err);
}

void unused_symbol(char* symbol, int decl_line, int decl_col) {
    Error err = (Error){.type = NOTE,
                        .code = UNUSED_SYMBOL,
                        .line = decl_line,
                        .col = decl_col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "unused symbol: '%s'", symbol);
    error_count[err.type]++;
    print_error(&err);
}

void unused_symbol_in_function(char* function, char* symbol, int decl_line, int decl_col) {
    Error err = (Error){.type = NOTE,
                        .code = UNUSED_SYMBOL,
                        .line = decl_line,
                        .col = decl_col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "unused symbol: '%s' in function '%s'", symbol, function);
    error_count[err.type]++;
    print_error(&err);
}

void assignation_error(char* symbol, char* dest_type, char* source_type, int decl_line, int decl_col) {
    Error err = (Error){.type = WARNING,
                        .code = ASSIGNATION_ERROR,
                        .line = decl_line,
                        .col = decl_col,
                        .has_line = true
                        };
    snprintf(err.message, ERROR_LEN,
             "trying to assign to '%s' of type '%s' a value of type '%s'",
             symbol, dest_type, source_type);
    error_count[err.type]++;
    print_error(&err);     
}

void error(ErrorType type, ErrorCode code, char* message) {
    Error err = (Error){.type = type, .code = code, .has_line = false};
    strcpy(err.message, message);
    error_count[err.type]++;
    print_error(&err);
}

bool fatal_error(void) {
    return error_count[ERROR];
}
