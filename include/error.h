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
    WRONG_PARAMETERS,
    USE_OF_UNDECLARE_SYMBOL,
    UNUSED_SYMBOL,
    ASSIGNATION_ERROR,
    REDEFINITION_OF_BUILTIN,
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
void init_error(const char* source);

void memory_error(void);
void error(ErrorType type, ErrorCode code, const char* message);
void already_declared_error(const char* symbol, int decl_line, int decl_col,
                                int last_decl_line);

void wrong_rtype_error(ErrorType type, const char* symbol, const char* current_type,
                       const char* expected_type, int decl_line, int decl_col);
void use_of_undeclare_symbol(const char* symbol, int decl_line, int decl_col);
void unused_symbol(const char* symbol, int decl_line, int decl_col);
void unused_symbol_in_function(const char* function, const char* symbol, int decl_line, int decl_col);
void assignation_error(const char* symbol, const char* dest_type, const char* source_type, int decl_line, int decl_col);
void redefinition_of_builtin_functions(const char* function, int decl_line, int decl_col);

/**
 * @brief Check if any errors has been registered
 * 
 * @return
 */
bool fatal_error(void);

#endif
