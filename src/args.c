#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>

#include "args.h"

Args init_args(void) {
    return (Args){.help = false,
                  .tree = false,
                  .err = false,
                  .symbols = false,
                  .source = NULL,
                  .name = NULL};
}

Args parse_args(int argc, char* argv[]) {
    Args args = init_args();
    int opt, opt_index = 0;
    static struct option long_options[] = {
        {"help",    no_argument,       0, 'h'},
        {"tree",    no_argument,       0, 't'},
        {"symtabs", no_argument,       0, 's'},
        {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, "htso:", long_options, &opt_index)) != -1) {
        switch (opt) {
            case 't':
                args.tree = true;
                break;
            case 'h':
                args.help = true;
                break;
            case 's':
                args.symbols = true;
                break;
            case '?':
                fprintf(stderr, "Unknown option : %c\n", optopt);
                args.err = true;
            default:
                break;
        }
    }
    if (optind < argc) {
        char* path = argv[optind];
        args.name = path;
        args.source = fopen(path, "r");
        if (!(args.source)) {
            fprintf(stderr, "Cannot open file '%s'\n", path);
            args.err = true;
        }
    }
    return args;
}

