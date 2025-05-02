#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "assembler.h"
#include "parser.h"
#include "logger.h"
#include "utils.h"
#include "code_generator.h"

/* Pointer to the head of the symbol table linked list */
extern Symbol *symbol_table;
extern MemoryWord *machine_code;
extern int machine_code_size;
extern int machine_code_capacity;

int IC = 100;
int DC = 0;

int *data_values = NULL;
int data_count = 0;
int data_capacity = 0;

void add_command(const ParsedLine *pline, int *IC) {
    int words = 1;

    if (pline->operand_count >= 1) {
        OperandType type = pline->operands[0].type;
        if (type != OPERAND_REGISTER_DIRECT) {
            words++;
        }
    }

    if (pline->operand_count == 2) {
        OperandType type = pline->operands[1].type;
        if (type != OPERAND_REGISTER_DIRECT) {
            words++;
        }
    }

    *IC += words;
}

int add_data_value(int value) {
    int *temp;
    if (data_count >= data_capacity) {
        int new_capacity = (data_capacity == 0) ? 64 : data_capacity * 2;
        temp = realloc(data_values, new_capacity * sizeof(int));
        if (!temp) return 0;
        data_values = temp;
        data_capacity = new_capacity;
    }

    data_values[data_count++] = value;
    return 1;
}

bool add_data(const ParsedLine *parsed, int *DC) {
    char *copy = NULL, *token = NULL, *args = NULL, *trimmed = NULL;
    int value, i;
    char *endptr;

    copy = strdup_c90(parsed->original_line);
    if (!copy) return 0;

    token = strtok(copy, " \t");  /* label or directive */
    if (!token) goto fail;

    if (strchr(token, ':')) {
        token = strtok(NULL, " \t");
        if (!token) goto fail;
    }

    if (strcmp(token, ".data") == 0) {
        args = strtok(NULL, "\n");
        if (!args) goto fail;

        token = strtok(args, ",");
        while (token) {
            trimmed = trim_whitespace(token);
            value = (int)strtol(trimmed, &endptr, 10);
            if (trimmed == endptr) goto fail;

            if (!add_data_value(value)) goto fail;
            (*DC)++;
            token = strtok(NULL, ",");
        }

        free(copy);
        return 1;
    }

    if (strcmp(token, ".string") == 0) {
        args = strtok(NULL, "\n");
        if (!args) goto fail;

        trimmed = trim_whitespace(args);
        i = (int)strlen(trimmed);
        if (i < 2 || trimmed[0] != '"' || trimmed[i - 1] != '"') goto fail;

        for (i = 1; trimmed[i] != '"' && trimmed[i] != '\0'; i++) {
            if (!add_data_value((int)trimmed[i])) goto fail;
            (*DC)++;
        }

        if (!add_data_value(0)) goto fail;
        (*DC)++;
        free(copy);
        return 1;
    }

fail:
    if (copy) free(copy);
    return 0;
}



bool first_pass(const char *filename) {
    FILE *file = fopen(filename, "r");
    char line[LINE_LENGTH + 2];
    int line_number = 0;
    bool has_error = false;
    IC = 100;
    DC = 0;

    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return false;
    }

    while (fgets(line, sizeof(line), file)) {
        ParsedLine parsed;
        line_number++;

        if (!parse_line(line, line_number, &parsed)) {
            asm_err(filename, line_number, 0, "%s", parsed.err_msg[0] ? parsed.err_msg : "Syntax error or invalid line.");
            has_error = true;
            continue;
        }

        switch (parsed.type) {
            case LINE_EMPTY:
            case LINE_COMMENT:
                break;

            case LINE_LABEL_ONLY:
                asm_err(filename, line_number, 0, "Label declared without a directive or instruction.");
                has_error = true;
                break;

            case LINE_DIRECTIVE:
                if (strstr(line, ".extern")) {
                    Symbol *existing = find_symbol(parsed.label);
                    if (existing) {
                        if (existing->type != SYMBOL_EXTERN) {
                            asm_err(filename, line_number, 0, "Symbol '%s' already defined; cannot redeclare as extern.", parsed.label);
                            has_error = true;
                            break;
                        }
                    } else {
                        add_symbol(parsed.label, 0, SYMBOL_EXTERN);
                    }
                    break;
                }

                if (strstr(line, ".entry")) {
                    break;
                }

                if (parsed.label[0] != '\0') {
                    Symbol *existing = find_symbol(parsed.label);
                    if (existing) {
                        asm_err(filename, line_number, 0, "Duplicate symbol '%s' declaration.", parsed.label);
                        has_error = true;
                        break;
                    }
                    add_symbol(parsed.label, DC, SYMBOL_DATA);
                }

                if (strstr(line, ".data") || strstr(line, ".string")) {
                    if (!add_data(&parsed, &DC)) {
                        asm_err(filename, line_number, 0, "Invalid .data or .string syntax.");
                        has_error = true;
                    }
                } else {
                    asm_err(filename, line_number, 0, "Unknown directive.");
                    has_error = true;
                }
                break;

            case LINE_COMMAND:
                if (parsed.label[0] != '\0') {
                    Symbol *existing = find_symbol(parsed.label);
                    if (existing) {
                        asm_err(filename, line_number, 0, "Duplicate label '%s'.", parsed.label);
                        has_error = true;
                        break;
                    }
                    add_symbol(parsed.label, IC, SYMBOL_CODE);
                }

                add_command(&parsed, &IC);
                break;

            case LINE_INVALID:
            default:
                asm_err(filename, line_number, 0, "Unrecognized or malformed line.");
                has_error = true;
                break;
        }
    }

    fclose(file);

    {
        Symbol *sym;
        for (sym = symbol_table; sym != NULL; sym = sym->next) {
            if (sym->type == SYMBOL_DATA) {
                sym->address += IC;
            }
        }
    }

    return !has_error;
}


bool second_pass(const char *filename) {
    FILE *file = fopen(filename, "r");
    char line[LINE_LENGTH + 2];
    int line_number = 0;
    int IC = 100;
    int original_IC = 100;
    bool has_error = false;

    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return false;
    }

    while (fgets(line, sizeof(line), file)) {
        ParsedLine parsed;
        int word_count, w, j;
        unsigned short words[MAX_WORDS_PER_LINE];

        line_number++;

        if (!parse_line(line, line_number, &parsed)) {
            continue;
        }

        switch (parsed.type) {
            case LINE_EMPTY:
            case LINE_COMMENT:
                break;

            case LINE_DIRECTIVE:
                if (strstr(line, ".entry")) {
                    Symbol *sym = find_symbol(parsed.label);
                    if (sym) {
                        sym->type = SYMBOL_ENTRY;
                    } else {
                        asm_err(filename, line_number, 0, "Unknown symbol in .entry: '%s'", parsed.label);
                        has_error = true;
                    }
                }
                break;

            case LINE_COMMAND:
                word_count = encode_instruction(&parsed, IC, words);

                for (w = 0; w < parsed.operand_count; w++) {
                    if (parsed.operands[w].type != OPERAND_REGISTER_DIRECT) {
                        if (encode_operand_word(&parsed.operands[w], IC + word_count, &words[word_count]) < 0) {
                            asm_err(filename, line_number, 0, "Undefined symbol '%s'", parsed.operands[w].value);
                            has_error = true;
                            break;
                        }
                        word_count++;
                    }
                }

                for (j = 0; j < word_count; j++) {
                    if (!add_machine_word(IC + j, words[j])) {
                        fprintf(stderr, "Memory allocation failed while writing machine code\n");
                        fclose(file);
                        return false;
                    }
                }

                IC += word_count;
                break;

            default:
                break;
        }
    }

    fclose(file);

    {
        int data_start = IC;
        int i;

        for (i = 0; i < data_count; i++) {
            if (!add_machine_word(data_start + i, (unsigned short)(data_values[i] & 0x3FFF))) {
                fprintf(stderr, "Memory allocation failed while writing data words\n");
                return false;
            }
        }
    }

    {
        char ob_name[FILENAME_MAX];
        FILE *ob_file;
        int i;

        snprintf(ob_name, sizeof(ob_name), "%s.ob", filename);
        ob_file = fopen(ob_name, "w");
        if (!ob_file) {
            fprintf(stderr, "Error: Cannot write to output file %s\n", ob_name);
            return false;
        }

        fprintf(ob_file, "%d %d\n", IC - original_IC, DC);

        for (i = 0; i < machine_code_size; i++) {
            fprintf(ob_file, "%06d %06x\n", machine_code[i].address, machine_code[i].value & 0x3FFF);
        }

        fclose(ob_file);
    }

    free(machine_code);
    machine_code = NULL;
    machine_code_size = 0;
    machine_code_capacity = 0;

    free(data_values);
    data_values = NULL;
    data_count = 0;
    data_capacity = 0;

    return !has_error;
}
