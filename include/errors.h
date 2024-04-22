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

typedef struct {                // error
    bool has_line;              // if error contains the line number where it
                                // was created
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
void error(ErrorType type, const char* message);
void already_declared_error(const char* symbol, int line, int col, int last_line);
void wrong_rtype_error(ErrorType type, const char* symbol, const char* current_type, const char* expected_type, int line, int col);
void use_of_undeclare_symbol(const char* symbol, int line, int col);
void unused_symbol(const char* symbol, int line, int col);
void unused_symbol_in_function(const char* function, const char* symbol, int line, int col);
void assignation_error(ErrorType type, const char* symbol, const char* dest_type, const char* source_type, int line, int col);
void redefinition_of_builtin_functions(const char* function, int line, int col);
void incorrect_array_access(const char* name, const char* access_type, int line, int col);
void invalid_operation(const char* operation, const char* type, int line, int col);

/**
 * @brief Check if any errors has been registered
 * 
 * @return
 */
bool fatal_error(void);

#endif
