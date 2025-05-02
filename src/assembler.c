#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "assembler.h"
#include "parser.h"
#include "logger.h"
#include "utils.h"

/* Pointer to the head of the symbol table linked list */
extern Symbol *symbol_table;

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

bool add_data(const char *line, int *DC) {
    const char *p;
    char *copy;
    char *token;
    char *endptr;
    int valid = 1;

    p = strstr(line, ".data");
    if (p != NULL) {
        p += 5;
        while (isspace(*p)) p++;
        if (*p == '\0') return 0;

        copy = malloc(strlen(p) + 1);
        if (!copy) return 0;
        strcpy(copy, p);

        token = strtok(copy, ",");
        while (token != NULL) {
            char *trimmed = trim_whitespace(token);
            if (*trimmed == '\0') {
                valid = 0;
                break;
            }

            strtol(trimmed, &endptr, 10);
            if (trimmed == endptr) {
                valid = 0;
                break;
            }

            (*DC)++;
            token = strtok(NULL, ",");
        }

        free(copy);
        return valid;
    }

    p = strstr(line, ".string");
    if (p != NULL) {
        p += 7;
        while (isspace(*p)) p++;
        if (*p == '\0') return 0;

        copy = malloc(strlen(p) + 1);
        if (!copy) return 0;
        strcpy(copy, p);

        copy = trim_whitespace(copy);
        if (copy[0] != '"' || copy[strlen(copy) - 1] != '"') {
            free(copy);
            return 0;
        }

        *DC += (int)(strlen(copy) - 2 + 1);
        free(copy);
        return 1;
    }

    return 0;
}


bool first_pass(const char *filename) {
    FILE *file = fopen(filename, "r");
    char line[LINE_LENGTH + 2];
    int line_number = 0;
    bool has_error = false;
    int IC = 100;
    int DC = 0;

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
                    if (!add_data(line, &DC)) {
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


/*
 * second_pass:
 * Re-parses the source file, resolves addresses, writes output files (.ob, .ent, .ext).
 */
bool second_pass(const char *filename) {
    return false;
}

