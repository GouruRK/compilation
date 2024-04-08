#include "sematic.h"

#include <stdbool.h>
#include <stdlib.h>

#include "table.h"
#include "error.h"

static void sort_table(Table* table) {
    qsort(table->array, table->cur_len, sizeof(Entry), compare_entries);
    table->sorted = true;
}

static void sort_collection(FunctionCollection* collection) {
    qsort(collection->funcs, collection->cur_len, sizeof(Function), compare_functions);
    collection->sorted = true;
}

static void sort_tables(Table* globals, FunctionCollection* collection) {
    sort_table(globals);
    sort_collection(collection);
    for (int i = 0; i < collection->cur_len; i++) {
        sort_table(&(collection->funcs[i].locals));
        sort_table(&(collection->funcs[i].parameters));
    }
}

char* return_type[] = {
    [CHAR] = "char",
    [INT] = "int",
    [VOID] = "void"
};

static void check_main(FunctionCollection* collection) {
    Function* start_fun = get_function(collection, "main");
    if (!start_fun) {
        error(ERROR, MAIN_MISSING, "no start function found");
        return;
    }
    if (start_fun->r_type != INT) {
        wrong_rtype_error("main", return_type[start_fun->r_type],
                              return_type[INT], start_fun->decl_line,
                              start_fun->decl_col);
        return;
    }
    if (start_fun->parameters.cur_len) {
        error(ERROR, WRONG_PARAMETERS, "main must take no parameters");
    }
}

int check_sem(Table* globals, FunctionCollection* collection, Node* tree) {
    sort_tables(globals, collection);
    check_main(collection);
    return 0;
}
