#include <stdio.h>
#include <stdbool.h>
#include "errors.h"

/* Helper function to display pass/fail */
void print_test_result(const char *name, bool passed) {
    printf("%s: %s\n", name, passed ? "PASS" : "FAIL");
}

int main() {
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

    return 0;
}
