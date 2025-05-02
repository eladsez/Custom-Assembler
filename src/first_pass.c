
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "logger.h"
#include "assembler.h"
#include "utils.h"
#include "parser.h"

void init_label_map(LabelMap *map) {
    map->count = 0;
    map->capacity = 8;
    map->entries = (LabelEntry *)malloc(map->capacity * sizeof(LabelEntry));
    if (!map->entries) {
        fprintf(stderr, "Error: Failed to allocate label map\n");
        exit(1);
    }
}

void insert_label(LabelMap *map, const char *name, int address) {
    int i;
    for (i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].name, name) == 0) {
            fprintf(stderr, "Error: Duplicate label '%s'\n", name);
            return;
        }
    }
    if (map->count == map->capacity) {
        map->capacity *= 2;
        map->entries = (LabelEntry *)realloc(map->entries, map->capacity * sizeof(LabelEntry));
        if (!map->entries) {
            fprintf(stderr, "Error: Failed to realloc label map\n");
            exit(1);
        }
    }
    strncpy(map->entries[map->count].name, name, LABEL_LENGTH - 1);
    map->entries[map->count].name[LABEL_LENGTH - 1] = '\0';
    map->entries[map->count].address = address;
    map->count++;
}

void free_label_map(LabelMap *map) {
    free(map->entries);
}

int is_label_definition(const char *line, char *label_out) {
    int i = 0;
    while (line[i] && !isspace(line[i]) && line[i] != ':') {
        label_out[i] = line[i];
        i++;
    }
    if (line[i] == ':') {
        label_out[i] = '\0';
        return 1;
    }
    return 0;
}

int count_words_for_data(char *line) {
    int count = 0;
    char *token = strtok(line, ",");
    while (token) {
        while (isspace(*token)) token++;
        if (*token != '\0') count++;
        token = strtok(NULL, ",");
    }
    return count;
}

int count_words_for_string(char *str_line) {
    char *start = strchr(str_line, '"');
    char *end = strrchr(str_line, '"');
    if (!start || !end || end <= start) return 0;
    return (end - start - 1) + 1;  
}

int count_instruction_words(char *line) {
    int count = 1; 
    char *token = strtok(line, " \t\n,");
    char *op1 = strtok(NULL, " \t\n,");
    char *op2 = strtok(NULL, " \t\n,");

    Operand operand1 = { OPERAND_NONE, "" };
    Operand operand2 = { OPERAND_NONE, "" };

    if (op1)
        operand1 = parse_operand(op1);
    if (op2)
        operand2 = parse_operand(op2);

    if (op1 && op2) {
        if (operand1.type != OPERAND_REGISTER_DIRECT)
            count++;
        if (operand2.type != OPERAND_REGISTER_DIRECT)
            count++;
    } else if (op1) {
        if (operand1.type != OPERAND_REGISTER_DIRECT)
            count++;
    }

    return count;
}

FILE *collect_labels_and_clean_file(const char *filepath, LabelMap *label_map) {
    FILE *source_file = fopen(filepath, "r");
    FILE *out_file;
    char out_path[512];
    char line[LINE_LENGTH];
    int IC = START_ADDRESS;
    int line_num = 0;

    if (!source_file) {
        fprintf(stderr, "Error: Cannot open source file %s\n", filepath);
        return NULL;
    }

    snprintf(out_path, sizeof(out_path), "%s.nolabels", filepath);
    out_file = fopen(out_path, "w");
    if (!out_file) {
        fprintf(stderr, "Error: Cannot create output file %s\n", out_path);
        fclose(source_file);
        return NULL;
    }

    while (fgets(line, LINE_LENGTH, source_file)) {
        char original_line[LINE_LENGTH];
        char label[LABEL_LENGTH];
        char line_copy[LINE_LENGTH];
        char *trim;
        int has_label = 0;
        line_num++;

        strcpy(original_line, line);
        strcpy(line_copy, line);

        trim = line;
        while (isspace(*trim)) trim++;
        if (*trim == ';' || *trim == '\0') {
            fputs(original_line, out_file);
            continue;
        }

        if (is_label_definition(trim, label)) {
            if (lookup_instruction(label) != INST_NONE) {
                asm_err(filepath, line_num, 1, "Label '%s' conflicts with an instruction name", label);
                fclose(source_file);
                fclose(out_file);
                remove(out_path);
                return NULL;
            }
            insert_label(label_map, label, IC);
            trim = strchr(trim, ':');
            if (trim) trim++;
            has_label = 1;
        }

        while (isspace(*trim)) trim++;

        if (strncmp(trim, ".data", 5) == 0) {
            char data_line[LINE_LENGTH];
            strcpy(data_line, trim + 5);
            IC += count_words_for_data(data_line);
        } else if (strncmp(trim, ".string", 7) == 0) {
            IC += count_words_for_string(trim + 7);
        } else if (strncmp(trim, ".extern", 7) == 0 || strncmp(trim, ".entry", 6) == 0) {

        } else {
            char inst_line[LINE_LENGTH];
            strcpy(inst_line, trim);
            IC += count_instruction_words(inst_line);
        }

        if (!has_label) {
            fputs(original_line, out_file);
        } else {
            fputs(trim, out_file);
        }
    }

    fclose(source_file);
    rewind(out_file);
    return out_file;
}
