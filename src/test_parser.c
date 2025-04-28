/* test_parser.c */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"

/* Helper function to display pass/fail */
void print_test_result(const char *name, bool passed) {
    printf("%s: %s\n", name, passed ? "PASS" : "FAIL");
}

int main() {
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

    return 0;
}

