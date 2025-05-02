#ifndef PRE_ASM_H
#define PRE_ASM_H

#include "assembler.h"

/* Constants */
#define TEMP_FILE_NAME "temp_pre_asm.am"
#define INITIAL_TABLE_SIZE 16

/* MacroEntry structure */
typedef struct MacroEntry {
    char *name;
    char *content;
    struct MacroEntry *next;
} MacroEntry;

/* MacroTable structure */
typedef struct {
    MacroEntry **buckets;
    size_t size;
    size_t count;
} MacroTable;

/* Function prototypes */
char *strdup_c90(const char *src);
unsigned int hash(const char *str, size_t table_size);
MacroTable *create_macro_table(void);
void insert_macro(MacroTable *table, const char *name, const char *content);
char *lookup_macro(MacroTable *table, const char *name);
void free_macro_table(MacroTable *table);
FILE *pre_assemble(const char *source_filename, MacroTable *table);

#endif /* PRE_ASM_H */