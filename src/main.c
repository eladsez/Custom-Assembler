#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "pre_asm.h"
#include "errors.h"
#include "logger.h"
#include "parser.h"

#define MAX_FILENAME_LEN 100


int pre_asm_test(int argc, char *argv[]){
    if (argc < 2) {
        printf("Usage: %s <input_file1> [input_file2 ...]\n", argv[0]);
        return 1;
    }
    int i;
    for (i = 1; i < argc; ++i) {
        const char *filename = argv[i];
        printf("Processing file: %s\n", filename);

        /* Create a new macro table for each file */
        MacroTable *table = create_macro_table();
        if (!table) {
            fprintf(stderr, "Failed to allocate macro table.\n");
            return 1;
        }

        /* Run the pre-assembler */
        FILE *output_file = pre_assemble(filename, table);
        if (!output_file) {
            fprintf(stderr, "Failed to preprocess %s\n", filename);
            free_macro_table(table);
            continue;
        }

        /* Print result to stdout */
        printf("Preprocessed output for %s:\n", filename);
        char line[LINE_LENGTH * 10];
        while (fgets(line, sizeof(line), output_file)) {
            fputs(line, stdout);
        }
        printf("\n");

        fclose(output_file);
        free_macro_table(table);
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

void test_parser_data(){
    ParsedLine parsed;
    bool result;

    /* Test cases for .data */
    result = parse_line(".data 5,,7", 1, &parsed);
    print_test_result(".data with double commas (should FAIL)", result == false);

    result = parse_line(".data 5, abc, 7", 2, &parsed);
    print_test_result(".data with non-numeric (should FAIL)", result == false);

    result = parse_line(".data 5,6,7", 3, &parsed);
    print_test_result(".data with valid numbers (should PASS)", result == true);

    result = parse_line(".data +5, -6, 7", 4, &parsed);
    print_test_result(".data with signed numbers (should PASS)", result == true);

    result = parse_line(".data  ", 5, &parsed);
    print_test_result(".data with no numbers (should FAIL)", result == false);
}

void test_parser(){
    bool result;
    ParsedLine parsed;

    /* Test parse_line for different types of lines */

    result = parse_line("mov r1, r2", 1, &parsed);
    print_test_result("parse_line (command)", result && parsed.type == LINE_COMMAND && parsed.operand_count == 2);

    result = parse_line(".data 5, -2, 100", 2, &parsed);
    print_test_result("parse_line (directive)", result && parsed.type == LINE_DIRECTIVE);

    result = parse_line("LABEL: mov r1, r2", 3, &parsed);
    print_test_result("parse_line (label + command)", result && parsed.type == LINE_COMMAND && strcmp(parsed.label, "LABEL") == 0);

    result = parse_line("; this is a comment", 4, &parsed);
    print_test_result("parse_line (comment)", result && parsed.type == LINE_COMMENT);

    result = parse_line("", 5, &parsed);
    print_test_result("parse_line (empty)", result && parsed.type == LINE_EMPTY);

    result = parse_line("rts", 6, &parsed);
    print_test_result("parse_line (no operands)", result && parsed.type == LINE_COMMAND && parsed.operand_count == 0);

    result = parse_line("invalid_instruction r1", 7, &parsed);
    print_test_result("parse_line (invalid instruction)", result == false);

    /* Additional test for invalid label */
    result = parse_line("1INVALID_LABEL: mov r1, r2", 8, &parsed);
    print_test_result("parse_line (invalid label)", result == false);

    /* Additional test for broken .data */
    result = parse_line(".data 5,,7", 9, &parsed);
    print_test_result("parse_line (broken .data)", result == false);
}

/*
 * Main program entry point.
 * Currently demonstrates logging functionality.
 * (The assembler logic is commented out but ready for future use.)
 */
int main(int argc, char *argv[]) {


    /* Example debug, info, warning, and error messages 
    log_dbg("Booting with revision: 0x%X", 0xFEEDBEEF);
    log_info("Init OK");
    log_warn("Sensor unstable");

    /* Example assembly warnings and errors tied to file locations
    asm_warn("src/end.asm", 8, 18, "unused variable 'temp'");
    asm_err("src/start.asm", 55, 12, "null pointer dereference");
    */

    pre_asm_test(argc, argv);

    return EXIT_SUCCESS;
}

