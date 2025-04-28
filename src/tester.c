#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "errors.h"
#include "logger.h"

#define TEST_FILE_COUNT 5

/* List of test input filenames */
const char *test_files[TEST_FILE_COUNT] = {
    "bad_instruction.as",
    "bad_label.as",
    "too_many_operands.as",
    "broken_data.as",
    "unclosed_string.as"
};

int main() {
    int i;

    for (i = 0; i < TEST_FILE_COUNT; i++) {
        printf("\n===== Testing file: %s =====\n", test_files[i]);

        reset_errors();

        if (!first_pass(test_files[i])) {
            /* Print all collected errors */
            print_errors();
        } else {
            printf("No errors detected in %s (unexpected)\n", test_files[i]);
        }
    }

    return 0;
}

