#include "types.h"

t_type set_type(t_type type, int flag) {
    return type | flag;
}

bool is_type(t_type type, int flag) {
    return type & flag;
}

t_type set_int(t_type type) {
    return set_type(type, T_INT);
}

t_type set_char(t_type type) {
    return set_type(type, T_CHAR);
}

t_type set_void(t_type type) {
    return set_type(type, T_VOID);
}

t_type set_array(t_type type) {
    return set_type(type, T_ARRAY);
}

t_type set_function(t_type type) {
    return set_type(type, T_FUNCTION);
}

bool is_int(t_type type) {
    return is_type(type, T_INT);
}

bool is_char(t_type type) {
    return is_type(type, T_CHAR);
}

bool is_void(t_type type) {
    return is_type(type, T_VOID);
}

bool is_array(t_type type) {
    return is_type(type, T_ARRAY);
}

bool is_function(t_type type) {
    return is_type(type, T_FUNCTION);
}