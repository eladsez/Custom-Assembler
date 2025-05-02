#include "pre_asm.h"
#include "errors.h"
#include "logger.h"
#include "parser.h"
#include "assembler.h"
#include <string.h>
#include <stdlib.h>

#define MAX_FILENAME_LEN 100


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

        char* expanded_file = pre_assemble(filename, table);
        if (!expanded_file) {
            fprintf(stderr, "Failed to preprocess %s\n", filename);
            free_macro_table(table);
            continue;
        }

        free_macro_table(table);

        /* Phase 2: Label Collection */
        LabelMap map;
        init_label_map(&map);
        FILE *nolabel_file = collect_labels_and_clean_file(expanded_file, &map);

        if (!nolabel_file) {
            fprintf(stderr, "Failed to collect labels and clean %s\n", filename);
            free_label_map(&map);
            continue;
        }

        printf("Label collection completed. Dumping label map:\n");
        int j;
        for (j = 0; j < map.count; j++) {
            printf("  %-20s -> %d\n", map.entries[j].name, map.entries[j].address);
        }

        free(expanded_file);
        fclose(nolabel_file);
        free_label_map(&map);
    }

    return 0;
}

/* Helper function to display pass/fail */
void print_test_result(const char *name, bool passed) {
    printf("%s: %s\n", name, passed ? "PASS" : "FAIL");
}

void test_errors(){
    bool result;

    /* Reset errors at the start */
    reset_errors();

    /* Initially there should be no errors */
    result = !has_errors();
    print_test_result("Initially no errors", result);

    /* Log a few errors */
    log_error(3, "Undefined label 'LOOP'");
    log_error(7, "Syntax error: unexpected comma");
    log_error(12, "Invalid opcode: 'movv'");

    /* Now there should be errors */
    result = has_errors();
    print_test_result("After logging errors", result);

    /* Print all collected errors */
    printf("\nPrinting all collected errors:\n");
    print_errors();

    /* Reset errors again */
    reset_errors();

    /* Check that errors have been cleared */
    result = !has_errors();
    print_test_result("After reset no errors", result);

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

