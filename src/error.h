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

#define MAX_ERRORS 256
#define ERROR_LEN 256

typedef enum {                  // differents types of errors
    WARNING,
    NOTE,
    EXCEPTION,
} ErrorType;

typedef enum {                  // different code of errors
    MEMORY_ERROR,
    ALREADY_DECLARE,        
    MAIN_MISSING,
    CONFLICT_TYPES
} ErrorCode;

typedef struct {                // error
    bool has_line;              // if error contains the line number where it
                                // was created
    ErrorCode code;             // error code
    ErrorType type;             // error type
    int line;                   // line where error was triggered
    char message[ERROR_LEN];    // associate message
} Error;

typedef struct {                // error collection
    bool exception;             // if at least one error is an exception
    int curlen;                 // current number of exceptions
    Error errs[MAX_ERRORS];     // collection of errors
} Errors;

/**
 * @brief Initialise error collection
 * 
 * @return collection
 */
Errors init_errors(void);

/**
 * @brief Add an error to the collection with the line number where
 *        the error was triggered
 * 
 * @param type error type
 * @param code error code
 * @param line triggered line
 * @param message associate message
 */
void add_line_error(ErrorType type, ErrorCode code, int line, char* message);

/**
 * @brief Add an error to the collection
 * 
 * @param type error type
 * @param code error code
 * @param message associate message
 */
void add_error(ErrorType type, ErrorCode code, char* message);

/**
 * @brief Print errors contains in the error collection
 * 
 */
void print_errors(void);

#endif