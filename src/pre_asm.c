#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pre_asm.h"
#include "logger.h"
#include "parser.h"

char *strdup_c90(const char *src) {
    char *copy;
    size_t len = strlen(src) + 1;
    copy = (char *)malloc(len);
    if (copy != NULL) {
        strcpy(copy, src);
    }
    return copy;
}

unsigned int hash(const char *str, size_t table_size) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++;
    }
    return hash % table_size;
}

MacroTable *create_macro_table(void) {
    MacroTable *table = (MacroTable *)malloc(sizeof(MacroTable));
    if (!table) return NULL;
    table->size = INITIAL_TABLE_SIZE;
    table->count = 0;
    table->buckets = (MacroEntry **)calloc(table->size, sizeof(MacroEntry *));
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    return table;
}

void insert_macro(MacroTable *table, const char *name, const char *content) {
    unsigned int index = hash(name, table->size);
    MacroEntry *new_entry = (MacroEntry *)malloc(sizeof(MacroEntry));
    if (!new_entry) return;
    new_entry->name = strdup_c90(name);
    new_entry->content = strdup_c90(content);
    new_entry->next = table->buckets[index];
    table->buckets[index] = new_entry;
    table->count++;
}

char *lookup_macro(MacroTable *table, const char *name) {
    unsigned int index = hash(name, table->size);
    MacroEntry *entry = table->buckets[index];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry->content;
        }
        entry = entry->next;
    }
    return NULL;
}

void free_macro_table(MacroTable *table) {
    size_t i;
    for (i = 0; i < table->size; ++i) {
        MacroEntry *entry = table->buckets[i];
        while (entry) {
            MacroEntry *next = entry->next;
            free(entry->name);
            free(entry->content);
            free(entry);
            entry = next;
        }
    }
    free(table->buckets);
    free(table);
}


char *pre_assemble(const char *source_filename, MacroTable *table) {
    FILE *source_file = fopen(source_filename, "r");
    FILE *temp_file;
    char *output_path;
    char line[LINE_LENGTH];
    int inside_macro = 0;
    char current_macro_name[LINE_LENGTH];
    char *macro_buffer = NULL;
    size_t buffer_size = 0;
    size_t content_length = 0;
    int line_num = 0;
    int had_error = 0;
    char *original_line;

    if (!source_file) {
        log_err("Error: Could not open source file %s\n", source_filename);
        return NULL;
    }

    /* Allocate .am filename */
    size_t len = strlen(source_filename);
    if (len < 3 || strcmp(source_filename + len - 3, ".as") != 0) {
        log_err("Error: Source file must end with .as\n");
        fclose(source_file);
        return NULL;
    }

    output_path = malloc(len + 1); 
    if (!output_path) {
        log_err("Error: Memory allocation failed for filename\n");
        fclose(source_file);
        return NULL;
    }

    snprintf(output_path, len, "%s", source_filename);
    strcpy(output_path + len - 2, "am");              

    temp_file = fopen(output_path, "w+");
    if (!temp_file) {
        log_err("Error: Could not create output file %s\n", output_path);
        free(output_path);
        fclose(source_file);
        return NULL;
    }

    while (fgets(line, LINE_LENGTH, source_file)) {
        char *trim;
        line_num++;
        original_line = strdup_c90(line);
        trim = line;
        while (isspace(*trim)) trim++;
        if (*trim == ';') {
            free(original_line);
            continue;
        }

        if (inside_macro) {
            char *macroend_pos = strstr(line, "macroend");
            if (macroend_pos != NULL) {
                char before_macroend[LINE_LENGTH];
                int i, found_token_before = 0, col;
                char *after_macroend;

                strncpy(before_macroend, line, macroend_pos - line);
                before_macroend[macroend_pos - line] = '\0';

                for (i = 0; before_macroend[i] != '\0'; i++) {
                    if (!isspace(before_macroend[i])) {
                        found_token_before = 1;
                        break;
                    }
                }

                if (found_token_before) {
                    col = (int)(macroend_pos - line) + 1;
                    asm_err(source_filename, line_num, col, "Unexpected token before 'macroend'");
                    had_error = 1;
                    free(original_line);
                    continue;
                }

                after_macroend = macroend_pos + strlen("macroend");
                while (isspace(*after_macroend)) after_macroend++;
                if (*after_macroend != '\0' && *after_macroend != ';') {
                    col = (int)(after_macroend - line) + 1;
                    asm_err(source_filename, line_num, col, "Unexpected token after 'macroend'");
                    had_error = 1;
                    free(original_line);
                    continue;
                }

                if (!macro_buffer) {
                    macro_buffer = malloc(1);
                    if (!macro_buffer) exit(1);
                    macro_buffer[0] = '\0';
                } else {
                    macro_buffer[content_length] = '\0';
                }

                insert_macro(table, current_macro_name, macro_buffer);
                inside_macro = 0;
                free(macro_buffer);
                macro_buffer = NULL;
                buffer_size = content_length = 0;
                free(original_line);
                continue;
            } else {
                size_t line_len = strlen(line);
                if (content_length + line_len + 1 >= buffer_size) {
                    buffer_size = (buffer_size + line_len + 1) * 2;
                    macro_buffer = realloc(macro_buffer, buffer_size);
                    if (!macro_buffer) exit(1);
                }
                strcpy(macro_buffer + content_length, line);
                content_length += line_len;
                free(original_line);
                continue;
            }
        }

        {
            char *macro_pos = strstr(line, "macro");
            if (macro_pos != NULL) {
                char before_macro[LINE_LENGTH];
                int found_token_before = 0, i, col;
                char *after_macro, *check;

                strncpy(before_macro, line, macro_pos - line);
                before_macro[macro_pos - line] = '\0';

                for (i = 0; before_macro[i] != '\0'; i++) {
                    if (!isspace(before_macro[i])) {
                        found_token_before = 1;
                        break;
                    }
                }

                if (found_token_before) {
                    col = (int)(macro_pos - line) + 1;
                    asm_err(source_filename, line_num, col, "Unexpected token before 'macro'");
                    had_error = 1;
                    free(original_line);
                    continue;
                }

                after_macro = macro_pos + strlen("macro");
                while (isspace(*after_macro)) after_macro++;

                i = 0;
                while (!isspace(after_macro[i]) && after_macro[i] != '\0' && i < LINE_LENGTH - 1)
                    i++;

                strncpy(current_macro_name, after_macro, i);
                current_macro_name[i] = '\0';

                if (INST_NONE != lookup_instruction(current_macro_name)) {
                    col = (int)(after_macro - line) + 1;
                    asm_err(source_filename, line_num, col, "Macro name '%s' conflicts with an instruction", current_macro_name);
                    had_error = 1;
                    free(original_line);
                    continue;
                }

                check = after_macro + i;
                while (isspace(*check)) check++;
                if (*check != '\0' && *check != ';') {
                    col = (int)(check - line) + 1;
                    asm_err(source_filename, line_num, col, "Unexpected token after macro name '%s'", current_macro_name);
                    had_error = 1;
                    free(original_line);
                    continue;
                }

                if (strlen(current_macro_name) > 0) {
                    log_dbg("Found macro definition: %s\n", current_macro_name);
                    inside_macro = 1;
                    macro_buffer = NULL;
                    buffer_size = content_length = 0;
                }

                free(original_line);
                continue;
            }
        }

        {
            char output_line[LINE_LENGTH * 10] = "";
            char code_part[LINE_LENGTH];
            char *semicolon_pos = strchr(line, ';');
            char *token;

            if (semicolon_pos) {
                strncpy(code_part, line, semicolon_pos - line);
                code_part[semicolon_pos - line] = '\0';
            } else {
                strncpy(code_part, line, LINE_LENGTH - 1);
                code_part[LINE_LENGTH - 1] = '\0';
            }

            token = strtok(code_part, " \t\n");
            while (token) {
                char *macro_content = lookup_macro(table, token);
                if (macro_content != NULL) {
                    if (macro_content[0] != '\0') {
                        strcat(output_line, macro_content);
                        if (macro_content[strlen(macro_content) - 1] != '\n')
                            strcat(output_line, " ");
                    }
                } else {
                    strcat(output_line, token);
                    strcat(output_line, " ");
                }
                token = strtok(NULL, " \t\n");
            }

            if (strlen(output_line) > 0) {
                size_t len = strlen(output_line);
                if (len > 0 && output_line[len - 1] == ' ')
                    output_line[len - 1] = '\0';
                strcat(output_line, "\n");
                fputs(output_line, temp_file);
            }

            free(original_line);
        }
    }

    fclose(source_file);
    if (had_error) {
        fclose(temp_file);
        remove(output_path);
        free(output_path);
        return NULL;
    }

    fclose(temp_file);
    return output_path;
}
