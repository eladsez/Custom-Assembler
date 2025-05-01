#ifndef PRE_ASM_H
#define PRE_ASM_H

#include <stdio.h>

/* Constants */
#define LINE_LENGTH 256
#define TEMP_FILE_NAME "temp_pre_asm.am"
#define HASH_TABLE_SIZE 100

/* MacroEntry structure */
typedef struct MacroEntry
{
    char *name;
    char *content;
    struct MacroEntry *next;
} MacroEntry;

/* External declaration of macro table */
extern MacroEntry *macro_table[HASH_TABLE_SIZE];

/* Function prototypes */
char *strdup_c90(const char *src);
unsigned int hash(const char *str);
void insert_macro(const char *name, const char *content);
char *lookup_macro(const char *name);
FILE *pre_assemble(const char *source_filename);

#endif /* PRE_ASM_H */

