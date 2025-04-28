
#include "errors.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define MAX_ERRORS 100
#define MAX_ERROR_MSG_LENGTH 128

/* Error structure to hold line number and message */
typedef struct {
    int line_number;
    char message[MAX_ERROR_MSG_LENGTH];
} Error;

/* Array to store errors */
static Error errors[MAX_ERRORS];
static int error_count = 0;

/*
 * log_error:
 * Logs a formatted error message associated with a specific line number.
 * Stores the error internally without printing immediately.
 */
void log_error(int line_number, const char *format, ...) {
    va_list args;
    if (error_count >= MAX_ERRORS) {
        return;
    }

    errors[error_count].line_number = line_number;

    va_start(args, format);
    vsprintf(errors[error_count].message, format, args);
    va_end(args);


    error_count++;
}

/*
 * has_errors:
 * Returns true if any errors were logged.
 */
bool has_errors() {
    return error_count > 0;
}

/*
 * reset_errors:
 * Clears all logged errors.
 */
void reset_errors() {
    error_count = 0;
}

/*
 * print_errors:
 * Prints all collected errors to stderr in a clean format.
 */
void print_errors() {
    int i;
    if (error_count == 0) {
        return;
    }

    fprintf(stderr, "Errors found:\n");
    for (i = 0; i < error_count; i++) {
        fprintf(stderr, "[line %d] %s\n", errors[i].line_number, errors[i].message);
    }
}

