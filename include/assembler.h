
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <stdbool.h>
#include "symbol_table.h"

#define START_ADDRESS 100
#define LINE_LENGTH 80
#define LABEL_LENGTH 31
#define MAX_FILENAME_LEN 512
#define MAX_WORDS_PER_LINE 3

/* Main passes */
bool first_pass(const char *filename);
bool second_pass(const char *filename);

#endif 

