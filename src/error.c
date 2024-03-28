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

static char* codes[] = {
    [MEMORY_ERROR] = "memory error",
    [ALREADY_DECLARE] = "symbol already declare",
    [MAIN_MISSING] = "missing starting function",
    [CONFLICT_TYPES] = "conflict types with"
};

static void add_default_error(Error err) {
    if (errors.curlen == MAX_ERRORS) {
        return;
    }
    errors.errs[errors.curlen] = err;
    errors.curlen++;
    errors.exception |= (err.type == ERROR);
}


void init_error(char* source) {
    errors.curlen = 0;
    errors.exception = false;
    errors.file = source;
}

void add_line_error(ErrorType type, ErrorCode code, int line, int col, char* message) {
    Error err = (Error){.has_line = true,
                        .code = code,
                        .type = type,
                        .line = line,
                        .col = col};
    strcpy(err.message, message);
    add_default_error(err);
}

void add_error(ErrorType type, ErrorCode code, char* message) {
    Error err = (Error){.has_line = false,
                        .code = code,
                        .type = type};
    strcpy(err.message, message);
    add_default_error(err);
}

void print_errors(void) {
    for (int i = 0; i < errors.curlen; i++) {
        ErrorCode code = errors.errs[i].code;
        ErrorType type = errors.errs[i].type;
        if (errors.errs[i].has_line) {
            fprintf(stderr, "%s:%d:%d - %s %s: %s\n",
                    errors.file, errors.errs[i].line, errors.errs[i].col, 
                    types[type], codes[code], errors.errs[i].message);
        } else {
            fprintf(stderr, "%s: %s %s: %s\n",
                    errors.file,
                    types[type], codes[code], errors.errs[i].message);
        }
    }
}

bool has_errors(void) {
    return errors.curlen;
}
