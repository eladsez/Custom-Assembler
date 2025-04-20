#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "errors.h"
#include "logger.h"

#define MAX_FILENAME_LEN 100

int main(int argc, char *argv[]) {
/*    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [file2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        char filename[MAX_FILENAME_LEN];
        snprintf(filename, MAX_FILENAME_LEN, "%s.as", argv[i]);

        printf("Assembling file: %s\n", filename);

        // Reset errors for each file
        reset_errors();

        // First pass: build symbol table, validate structure
        if (!first_pass(filename)) {
            fprintf(stderr, "First pass failed for %s\n", filename);
            continue;
        }

        // Second pass: generate output files
        if (!second_pass(filename)) {
            fprintf(stderr, "Second pass failed for %s\n", filename);
            continue;
        }

        printf("Successfully assembled %s\n", filename);
    }
*/
    
    LOG_DBG("Booting with revision: 0x%X", 0xFEEDBEEF);
    LOG_INFO("Init OK");
    LOG_WRN("Sensor unstable");
    LOG_ERR("Unhandled exception");

    ASM_WRN("src/end.asm", 8, 18, "unused variable ‘temp’");
    ASM_ERR("src/start.asm", 55, 12, "null pointer dereference");

    return EXIT_SUCCESS;
}
