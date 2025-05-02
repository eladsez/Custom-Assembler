#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "code_generator.h"
#include "symbol_table.h"
#include "logger.h"
#include "pre_asm.h"

#define MAX_FILENAME 256

extern Symbol *symbol_table;

int main(int argc, char *argv[]) {
    int i;

    if (argc < 2) {
        printf("Usage: %s <file1> [file2 ...] (without .as extension)\n", argv[0]);
        return 1;
    }

    for (i = 1; i < argc; ++i) {
        char input_filename[MAX_FILENAME];
        char *expanded_file;
        Symbol *sym;
        MacroTable *table;

        snprintf(input_filename, sizeof(input_filename), "%s.as", argv[i]);

        table = create_macro_table();
        if (!table) {
            fprintf(stderr, "Failed to allocate macro table.\n");
            continue;
        }

        expanded_file = pre_assemble(input_filename, table);
        free_macro_table(table);

        if (!expanded_file) {
            fprintf(stderr, "Failed to preprocess %s\n", input_filename);
            continue;
        }

        if (!first_pass(expanded_file)) {
            fprintf(stderr, "First pass failed for %s\n", expanded_file);
            free(expanded_file);
            free_symbol_table();
            continue;
        }

    
        if (!second_pass(expanded_file)) {
            fprintf(stderr, "Second pass failed for %s\n", expanded_file);
            free(expanded_file);
            free_symbol_table();
            continue;
        }

        free(expanded_file);
        free_symbol_table();
    }

    return 0;
}
