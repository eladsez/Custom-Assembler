#include "pre_asm.h"
#include "logger.h"
#include "parser.h"
#include "assembler.h"
#include <string.h>
#include <stdlib.h>

extern Symbol *symbol_table;

int pre_asm_test(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file1> [input_file2 ...]\n", argv[0]);
        return 1;
    }
    int i;
    
    for (i = 1; i < argc; ++i) {
        char *filename = argv[i];
        printf("Processing file: %s\n", filename);

        /* Phase 1: Macro Expansion */
        MacroTable *table = create_macro_table();
        if (!table) {
            fprintf(stderr, "Failed to allocate macro table.\n");
            return 1;
        }

        char *expanded_file = pre_assemble(filename, table);
        if (!expanded_file) {
            fprintf(stderr, "Failed to preprocess %s\n", filename);
            free_macro_table(table);
            continue;
        }

        free_macro_table(table);

        /* Phase 2: First Pass */
        printf("Running first pass on: %s\n", expanded_file);
        bool pass_ok = first_pass(expanded_file);
        if (!pass_ok) {
            fprintf(stderr, "First pass failed for %s\n", expanded_file);
            free(expanded_file);
            free_symbol_table();
            continue;
        }

        /* Dump Symbol Table */
        printf("Symbol table after first pass for %s:\n", expanded_file);
        Symbol *sym = symbol_table;
        while (sym) {
            const char *type_str = 
                (sym->type == SYMBOL_CODE) ? "CODE" :
                (sym->type == SYMBOL_DATA) ? "DATA" :
                (sym->type == SYMBOL_EXTERN) ? "EXTERN" :
                (sym->type == SYMBOL_ENTRY) ? "ENTRY" : "UNKNOWN";

            printf("  Name: %-20s Address: %-5d Type: %s\n", sym->name, sym->address, type_str);
            sym = sym->next;
        }

        /* Cleanup for next file */
        free(expanded_file);
        free_symbol_table();
        printf("\n");
    }

    return 0;
}


/* Helper function to display pass/fail */
void print_test_result(const char *name, bool passed) {
    printf("%s: %s\n", name, passed ? "PASS" : "FAIL");
}
/*
 * Main program entry point.
 * Currently demonstrates logging functionality.
 * (The assembler logic is commented out but ready for future use.)
 */
int main(int argc, char *argv[]) {

    pre_asm_test(argc, argv);

    return EXIT_SUCCESS;
}

