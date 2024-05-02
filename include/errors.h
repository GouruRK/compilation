#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

#include "types.h"

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

/**
 * @brief Print a memory error message
 * 
 */
void memory_error(void);

/**
 * @brief Print an error message when a symbol is already declared
 * 
 * @param symbol symbol already declared
 * @param line line where the error is triggered
 * @param col column where the error is triggered
 * @param last_line line where the symbol was previously defined
 */
void already_declared_error(const char* symbol, int line, int col, int last_line);

/**
 * @brief Print an error message when a function hasn't its presume return type
 *        (for function like 'main') or when a return expression type is the
 *        wrong one
 * 
 * @param type type of errors
 * @param symbol function name
 * @param current_type actual return type
 * @param expected_type presumed type in function declaration
 * @param line line where the error is triggered
 * @param col column where the error is triggered
 */
void wrong_rtype_error(ErrorType type, const char* symbol,
                       t_type current_type, t_type expected_type,
                       int line, int col);

/**
 * @brief Print an error message when using a symbol which is not is the
 *        symbol tables
 * 
 * @param symbol undefined symbol
 * @param line line where the error is triggered
 * @param col column where the error is triggered
 */
void use_of_undeclare_symbol(const char* symbol, int line, int col);

/**
 * @brief Print a message when a symbol is not used
 * 
 * @param symbol unused symbol
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void unused_symbol(const char* symbol, int line, int col);

/**
 * @brief Print a message when a symbol inside a function is not used
 * 
 * @param function function where the symbol is declared
 * @param symbol unused symbol
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void unused_symbol_in_function(const char* function, const char* symbol, int line, int col);

/**
 * @brief Print a message when assignation is incorrect
 * 
 * @param type type of errors
 * @param symbol name of variable
 * @param dest_type type of the assignation
 * @param source_type type of variable symbol
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void assignation_error(ErrorType type, const char* symbol, t_type dest_type, t_type source_type, int line, int col);

/**
 * @brief Print a message when a function redefine a builtin function
 * 
 * @param function array of functions
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void redefinition_of_builtin_functions(const char* function, int line, int col);

/**
 * @brief Print a message when index is incorrect
 * 
 * @param name name of array
 * @param access_type type of the access index
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void incorrect_array_access(const char* name, t_type access_type, int line, int col);

/**
 * @brief Print a message when operation with invalid types
 * 
 * @param operation symbol of operation
 * @param type type
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void invalid_operation(const char* operation, t_type type, int line, int col);

/**
 * @brief Print a message when invalid condition
 * 
 * @param type condition type at the moment
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void invalid_condition(t_type type, int line, int col);

/**
 * @brief Print a message when a type check occurs on non int or char values 
 * 
 * @param function name of function
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void incorrect_function_call(const char* function, int line, int col);

/**
 * @brief Print a message when a actual parameter is different that type of function's parameter
 * 
 * @param type type of errors
 * @param function name of function
 * @param param_name name of parameter
 * @param expected type expected of parameter
 * @param current type of parameter
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void invalid_parameter_type(ErrorType type, const char* function,
                            const char* param_name, t_type expected,
                            t_type current, int line, int col);

/**
 * @brief Print a message when type symbol is incorrect than expected type
 * 
 * @param symbol name of symbol
 * @param sym_type type of symbol
 * @param expected_type expected type
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void incorrect_symbol_use(const char* symbol, t_type sym_type, t_type expected_type, int line, int col);

/**
 * @brief Print a message when declaration of a array is incorrect
 * 
 * @param symbol name of symbol
 * @param line line where the message is triggered
 * @param col column where the message is triggered
 */
void incorrect_array_decl(const char* symbol, int line, int col);

/**
 * @brief Print custom message
 * 
 * @param type error type
 * @param message custom message
 */
void error(ErrorType type, const char* message);

/**
 * @brief Print custom message with custom line and column
 * 
 * @param type error type
 * @param message custom message
 * @param line custom line
 * @param col custom column
 */
void line_error(ErrorType type, const char* message, int line, int col);

/**
 * @brief Check if any errors has been registered
 * 
 * @return
 */
bool fatal_error(void);

#endif
