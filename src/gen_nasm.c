#include "gen_nasm.h"

#include <stdio.h>

static FILE* out;

static void write_init(int globals_size) {
    fprintf(out, "extern putchar\n"
                 "extern getchar\n"
                 "extern putint\n"
                 "extern getint\n"
                 "\nglobal _start\n\n"
                 "section .bss\n"
                 "globals: resb %d\n"
                 "\n_start:\n", 
                 globals_size);
}

static void write_exit(int code) {
    fprintf(out, "mov rax, 60\n"
                 "mov rdi, %d\n"
                 "syscall\n\n",
                 code);
}

static int create_file(char* output) {
    if (!output) {
        output = "prog";
    }
    char filename[64];
    snprintf(filename, 64, "%s.asm", output);

    out = fopen(filename, "w");
    return out != NULL;
}



void gen_nasm(char* output, Table* globals, FunctionCollection* collection, Node* tree) {
    if (!create_file(output)) {
        return;
    }
    write_init(globals->cur_len); // TODO: replace this value by the number of bytes used for globals

    // ...

    write_exit(0);
    fclose(out);
}
