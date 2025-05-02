
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <stdbool.h>

#define MAX_SYMBOL_NAME 32
#define START_ADDRESS 100
#define LINE_LENGTH 80
#define LABEL_LENGTH 31

/* Label entry definition */
typedef struct {
    char name[LABEL_LENGTH];
    int address;
} LabelEntry;

/* Dynamic label map */
typedef struct {
    LabelEntry *entries;
    int count;
    int capacity;
} LabelMap;

void init_label_map(LabelMap *map);
void free_label_map(LabelMap *map);
FILE *collect_labels_and_clean_file(const char *filepath, LabelMap *label_map);



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

