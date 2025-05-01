/* TODO nedds to check that macro name is not like another action name  */
/* pre_asm.c - Function for pre-assembling a file and searching for "macro" */
/* Written in C90 standard */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pre_asm.h"

/* Constants declared in pre_asm.h */

/* Structure declared in pre_asm.h */    struct MacroEntry *next;
} MacroEntry;

/* Hash table */
MacroEntry *macro_table[HASH_TABLE_SIZE];

/* Custom strdup for C90 compatibility */
char *strdup_c90(const char *src)
{
    char *copy;
    size_t len = strlen(src) + 1;
    copy = (char *)malloc(len);
    if (copy != NULL)
    {
        strcpy(copy, src);
    }
    return copy;
}

/* Simple hash function */
unsigned int hash(const char *str)
{
    unsigned int hash = 0;
    while (*str)
    {
        hash = (hash << 5) + *str++;
    }
    return hash % HASH_TABLE_SIZE;
}

/* Insert macro name and content into hash table */
void insert_macro(const char *name, const char *content)
{
    unsigned int index = hash(name);
    MacroEntry *new_entry = (MacroEntry *)malloc(sizeof(MacroEntry));
    if (new_entry == NULL)
    {
        printf("Error: Memory allocation failed.\n");
        return;
    }
    new_entry->name = strdup_c90(name);
    new_entry->content = strdup_c90(content);
    new_entry->next = macro_table[index];
    macro_table[index] = new_entry;
}

/* Lookup a macro by name */
char *lookup_macro(const char *name)
{
    unsigned int index = hash(name);
    MacroEntry *entry = macro_table[index];
    while (entry != NULL)
    {
        if (strcmp(entry->name, name) == 0)
        {
            return entry->content;
        }
        entry = entry->next;
    }
    return NULL;
}

FILE *pre_assemble(const char *source_filename)
{
    FILE *source_file;
    FILE *temp_file;
    char line[LINE_LENGTH];
    char *macro_pos;
    int inside_macro = 0;
    char current_macro_name[LINE_LENGTH];
    char current_macro_content[LINE_LENGTH * 100];

    current_macro_content[0] = '\0';

    source_file = fopen(source_filename, "r");
    if (source_file == NULL)
    {
        printf("Error: Could not open source file %s\n", source_filename);
        return NULL;
    }

    temp_file = fopen(TEMP_FILE_NAME, "w+");
    if (temp_file == NULL)
    {
        printf("Error: Could not create temporary file.\n");
        fclose(source_file);
        return NULL;
    }

    while (fgets(line, LINE_LENGTH, source_file) != NULL)
    {
        /* Skip full-line comments */
        {
            char *trim = line;
            while (isspace(*trim)) trim++;
            if (*trim == ';') continue;
        }

        if (inside_macro)
        {
            if (strstr(line, "macroend") != NULL)
            {
                insert_macro(current_macro_name, current_macro_content);
                inside_macro = 0;
            }
            else
            {
                strcat(current_macro_content, line);
            }
            continue;
        }

        macro_pos = strstr(line, "macro");
        if (macro_pos != NULL)
        {
            int i = 0;
            char *start = macro_pos + strlen("macro");
            while (isspace(*start)) start++;
            while (!isspace(*start) && *start != '\0' && i < LINE_LENGTH - 1)
            {
                current_macro_name[i++] = *start++;
            }
            current_macro_name[i] = '\0';

            if (strlen(current_macro_name) > 0)
            {
                printf("Found macro definition: %s\n", current_macro_name);
                current_macro_content[0] = '\0';
                inside_macro = 1;
            }
            continue;
        }

        {
            char output_line[LINE_LENGTH * 10];
            char code_part[LINE_LENGTH];
            char *semicolon_pos;
            char *token;

            output_line[0] = '\0';
            code_part[0] = '\0';

            semicolon_pos = strchr(line, ';');
            if (semicolon_pos != NULL)
            {
                strncpy(code_part, line, semicolon_pos - line);
                code_part[semicolon_pos - line] = '\0';
            }
            else
            {
                strncpy(code_part, line, LINE_LENGTH - 1);
                code_part[LINE_LENGTH - 1] = '\0';
            }

            token = strtok(code_part, " \t\n");
            while (token != NULL)
            {
                char *macro_content = lookup_macro(token);
                if (macro_content != NULL)
                {
                    strcat(output_line, macro_content);
                    if (macro_content[strlen(macro_content) - 1] != '\n')
                    {
                        strcat(output_line, " ");
                    }
                }
                else
                {
                    strcat(output_line, token);
                    strcat(output_line, " ");
                }
                token = strtok(NULL, " \t\n");
            }

            /* Write to temp file only if there's actual code */
            if (strlen(output_line) > 0)
            {
                size_t len = strlen(output_line);
                if (len > 0 && output_line[len - 1] == ' ')
                {
                    output_line[len - 1] = '\0';
                }
                strcat(output_line, "\n");
                fputs(output_line, temp_file);
            }
        }
    }

    fclose(source_file);
    rewind(temp_file);
    return temp_file;
}

int main(int argc, char *argv[])
{
    FILE *processed_file;
    char line[LINE_LENGTH];

    if (argc != 2)
    {
        printf("Usage: %s <source_file.as>\n", argv[0]);
        return 1;
    }

    processed_file = pre_assemble(argv[1]);
    if (processed_file == NULL)
    {
        return 1;
    }

    while (fgets(line, LINE_LENGTH, processed_file) != NULL)
    {
        printf("%s", line);
    }

    fclose(processed_file);
    return 0;
}

