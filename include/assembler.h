#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdbool.h>

/**
 * Performs the first pass on the given assembly source file.
 * Builds the symbol table, validates labels, and calculates IC/DC.
 *
 * @param filename The name of the source file (.as)
 * @return true if no errors occurred, false otherwise
 */
bool first_pass(const char *filename);

/**
 * Performs the second pass on the given assembly source file.
 * Generates object (.ob), entry (.ent), and external (.ext) files.
 *
 * @param filename The name of the source file (.as)
 * @return true if no errors occurred, false otherwise
 */
bool second_pass(const char *filename);

#endif
