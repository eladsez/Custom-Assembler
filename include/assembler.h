
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdbool.h>

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

/* Global counters */
extern int IC;
extern int DC;

/* Main passes */
bool first_pass(const char *filename);
bool second_pass(const char *filename);

#endif 

