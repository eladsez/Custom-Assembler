

#ifndef LABEL_SYM_H
#define LABEL_SYM_H

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOL_NAME 32

/* Symbol types */
typedef enum {
    SYMBOL_CODE,
    SYMBOL_DATA,
    SYMBOL_EXTERN,
    SYMBOL_ENTRY
} SymbolType;

/* Symbol structure */
typedef struct Symbol {
    char name[MAX_SYMBOL_NAME];
    int address;
    SymbolType type;
    struct Symbol *next;
} Symbol;


void free_symbol_table();

void add_symbol(const char *name, int address, SymbolType type);

Symbol* find_symbol(const char *name);

void mark_entry(const char *name);

#endif