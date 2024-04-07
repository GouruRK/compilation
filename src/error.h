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
#define ERROR_LEN 512

typedef enum {                  // differents types of errors
    WARNING,
    NOTE,
    ERROR,
} ErrorType;

typedef enum {                  // different code of errors
    MEMORY_ERROR,
    ALREADY_DECLARE,        
    MAIN_MISSING,
    WRONG_RTYPE,
    WRONG_PARAMETERS
} ErrorCode;

typedef struct {                // error
    bool has_line;              // if error contains the line number where it
                                // was created
    ErrorCode code;             // error code
    ErrorType type;             // error type
    int line;                   // line where the error was triggered
    int col;                    // column where the error was triggered
    char message[ERROR_LEN];    // associate message
} Error;

/**
 * @brief Initiate error collection
 * 
 * @param source current parsed file
 */
void init_error(char* source);

void memory_error(void);
void error(ErrorType type, ErrorCode code, char* message);
void already_declared_error(char* symbol, int decl_line, int decl_col,
                                int last_decl_line);

void wrong_rtype_error(char* symbol, char* current_type,
                           char* expected_type, int decl_line, int decl_col);

/**
 * @brief Check if any errors has been registered
 * 
 * @return
 */
bool fatal_error(void);

#endif
