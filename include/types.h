#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

typedef int t_type; 

#define T_NONE      0       // 0 << 0 - 0000 0001 
#define T_INT       2       // 1 << 1 - 0000 0010 
#define T_CHAR      4       // 1 << 2 - 0000 0100 
#define T_VOID      8       // 1 << 3 - 0000 1000 
#define T_ARRAY    16       // 1 << 4 - 0001 0000 
#define T_FUNCTION 32       // 1 << 5 - 0010 0000 

t_type set_type(t_type type, int flag);
bool is_type(t_type type, int flag);

t_type set_int(t_type type);
t_type set_char(t_type type);
t_type set_void(t_type type);
t_type set_array(t_type type);
t_type set_function(t_type type);

bool is_int(t_type type);
bool is_char(t_type type);
bool is_void(t_type type);
bool is_array(t_type type);
bool is_function(t_type type);

// typedef enum {
//     T_INT, T_CHAR, T_VOID, T_ARRAY, T_FUNCTION, T_NONE
// } Types;

#endif
