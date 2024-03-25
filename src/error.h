#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

// colors
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define PURPLE "\x1b[35m"
#define CYAN "\x1b[36m"
#define UNDERLINE "\e[4m"
#define BOLD "\e[1m"
#define RESET "\x1b[0m"


#define ERRORS_LEN 256
#define ERROR_LEN 256

typedef enum {
    WARNING,
    NOTE,
    EXCEPTION,
} ErrorType;

typedef enum {
    MEMORY_ERROR,
    ALREADY_DECLARE,
    MAIN_MISSING,
    CONFLICT_TYPES
} ErrorCode;

typedef struct {
    bool has_line;
    ErrorCode code;
    ErrorType type;
    int line;
    char message[ERROR_LEN];
} Error;

typedef struct {
    bool exception;
    int curlen;
    Error errs[ERRORS_LEN];
} Errors;

Errors init_errors(void);
void add_line_error(ErrorType type, ErrorCode code, int line, char* message);
void add_error(ErrorType type, ErrorCode code, char* message);
void print_errors(void);

#endif
