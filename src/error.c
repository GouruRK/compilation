#include "error.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern Errors errors;

static char* types[] = {
    [WARNING] = PURPLE "warning" RESET,
    [NOTE] = CYAN "note" RESET,
    [EXCEPTION] = RED "exception" RESET
};

static char* codes[] = {
    [MEMORY_ERROR] = "memory error",
    [ALREADY_DECLARE] = "symbol already declare",
    [MAIN_MISSING] = "missing starting function",
    [CONFLICT_TYPES] = "conflict types with"
};

Errors init_errors(void) {
    return (Errors){.curlen = 0, .exception = false};
}

static void add_default_error(Error err) {
    errors.errs[errors.curlen] = err;
    errors.curlen++;
    errors.exception |= (err.type == EXCEPTION);
}

void add_line_error(ErrorType type, ErrorCode code, int line, char* message) {
    Error err = (Error){.has_line = true,
                        .code = code,
                        .type = type,
                        .line = line};
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
            fprintf(stderr, "%s at line %d, %s: %s\n",
                    types[type], errors.errs[i].line,
                    codes[code], errors.errs[i].message);
        } else {
            fprintf(stderr, "%s, %s: %s\n",
                    types[type], codes[code], errors.errs[i].message);
        }
    }
}
