#include "error.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

Errors errors;

bool is_init = false;

static char* types[] = {
    [WARNING] = PURPLE "warning" RESET,
    [NOTE] = CYAN "note" RESET,
    [ERROR] = RED "error" RESET
};

static void add_default_error(Error err) {
    if (errors.curlen == MAX_ERRORS) {
        return;
    }
    errors.errs[errors.curlen] = err;
    errors.curlen++;
    errors.fatal |= (err.type == ERROR);
}

void init_error(char* source) {
    errors.curlen = 0;
    errors.fatal = false;
    errors.file = source;
}

void add_memory_error(void) {
    Error err = (Error){.type = ERROR,
                        .code = MEMORY_ERROR,
                        .has_line = false};
    strcpy(err.message, "error while allocating memory");
    add_default_error(err);
}

void add_already_declared_error(char* symbol, int decl_line, int decl_col,
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
    add_default_error(err);
}

void add_wrong_rtype_error(char* symbol, char* current_type,
                           char* expected_type, int decl_line, int decl_col) {
    Error err = (Error){.type = WARNING,
                        .code = WRONG_RTYPE,
                        .line = decl_line,
                        .col  = decl_col,
                        .has_line = true};
    snprintf(err.message, ERROR_LEN,
             "'%s' return type must be '%s' instead of '%s'",
             symbol, expected_type, current_type);
    add_default_error(err);
}

void add_error(ErrorType type, ErrorCode code, char* message) {
    Error err = (Error){.type = type, .code = code, .has_line = false};
    strcpy(err.message, message);
    add_default_error(err);
}

void print_errors(void) {
    for (int i = 0; i < errors.curlen; i++) {
        Error error = errors.errs[i]; 
        if (errors.errs[i].has_line) {
            fprintf(stderr, "%s:%d:%d %s: %s\n",
                    errors.file, error.line, error.col, 
                    types[error.type], error.message);
        } else {
            fprintf(stderr, "%s: %s: %s\n",
                    errors.file, types[error.type], errors.errs[i].message);
        }
    }
}

bool has_errors(void) {
    return errors.curlen;
}
