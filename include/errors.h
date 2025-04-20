#ifndef ERRORS_H
#define ERRORS_H

#include <stdbool.h>

/**
 * Logs an error message for a specific line in the source file.
 *
 * @param line_number The line number where the error occurred
 * @param format printf-style format string for the error message
 * @param ... Additional arguments for the format string
 */
void log_error(int line_number, const char *format, ...);

/**
 * Returns true if any errors were logged.
 *
 * @return true if errors exist, false otherwise
 */
bool has_errors();

/**
 * Clears all previously logged errors.
 * Should be called before starting a new file.
 */
void reset_errors();

/**
 * Prints all collected errors to stderr.
 */
void print_errors();

#endif
