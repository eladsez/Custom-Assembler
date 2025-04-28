/* test_parser_data.c */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"

/* Helper function to print pass/fail */
void print_test(const char *desc, bool passed) {
    printf("%s: %s\n", desc, passed ? "PASS" : "FAIL");
}

int main() {
    ParsedLine parsed;
    bool result;

    /* Test cases for .data */
    result = parse_line(".data 5,,7", 1, &parsed);
    print_test(".data with double commas (should FAIL)", result == false);

    result = parse_line(".data 5, abc, 7", 2, &parsed);
    print_test(".data with non-numeric (should FAIL)", result == false);

    result = parse_line(".data 5,6,7", 3, &parsed);
    print_test(".data with valid numbers (should PASS)", result == true);

    result = parse_line(".data +5, -6, 7", 4, &parsed);
    print_test(".data with signed numbers (should PASS)", result == true);

    result = parse_line(".data  ", 5, &parsed);
    print_test(".data with no numbers (should FAIL)", result == false);

    /* Placeholder for future .string tests */
    
    return 0;
}

