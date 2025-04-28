#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "assembler.h"
#include "parser.h"
#include "errors.h"
#include "logger.h"

/* Instruction and Data Counters */
int IC = 100;
int DC = 0;

/* Pointer to the head of the symbol table linked list */
static Symbol *symbol_table = NULL;

/*
 * add_symbol:
 * Adds a new symbol to the symbol table if it does not already exist.
 */
void add_symbol(const char *name, int address, SymbolType type) {
    Symbol *existing = symbol_table;
    while (existing) {
        if (strcmp(existing->name, name) == 0) {
            /* Duplicate symbol, do not add */
            return;
        }
        existing = existing->next;
    }
    {
        Symbol *new_sym = malloc(sizeof(Symbol));
        if (new_sym) {
            strncpy(new_sym->name, name, MAX_SYMBOL_NAME);
            new_sym->address = address;
            new_sym->type = type;
            new_sym->next = symbol_table;
            symbol_table = new_sym;
        }
    }
}

/*
 * find_symbol:
 * Searches the symbol table for a given name and returns the symbol if found.
 */
Symbol* find_symbol(const char *name) {
    Symbol *curr = symbol_table;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/*
 * mark_entry:
 * Marks an existing symbol as an entry type.
 */
void mark_entry(const char *name) {
    Symbol *sym = find_symbol(name);
    if (sym) {
        sym->type = SYMBOL_ENTRY;
    }
}

/*
 * first_pass:
 * Parses the source file line-by-line, builds the symbol table,
 * calculates IC and DC values, and logs syntax errors.
 */
bool first_pass(const char *filename) {
    FILE *fp;
    char line[256];
    int line_num;

    fp = fopen(filename, "r");
    if (!fp) {
        LOG_ERR("Cannot open file: %s", filename);
        return false;
    }

    line_num = 0;
    while (fgets(line, sizeof(line), fp)) {
        ParsedLine parsed;

        line_num++;
        if (!parse_line(line, line_num, &parsed)) {
            log_error(line_num, "Syntax error");
            continue;
        }

        /* Handle labels for commands or data */
        if (parsed.type == LINE_COMMAND && parsed.label[0]) {
            add_symbol(parsed.label, IC, SYMBOL_CODE);
        } else if (parsed.type == LINE_DIRECTIVE && parsed.label[0]) {
            if (strstr(line, ".data") || strstr(line, ".string")) {
                add_symbol(parsed.label, DC, SYMBOL_DATA);
            }
        } else if (parsed.type == LINE_DIRECTIVE && strstr(line, ".extern")) {
            char extern_label[MAX_SYMBOL_NAME];
            sscanf(line, ".extern %s", extern_label);
            add_symbol(extern_label, 0, SYMBOL_EXTERN);
        }

        /* Update instruction or data counters */
        if (parsed.type == LINE_COMMAND) {
            IC += 1 + parsed.operand_count;
        } else if (parsed.type == LINE_DIRECTIVE && strstr(line, ".data")) {
            const char *p = strchr(line, ' ');
            while (p && *p) {
                if (isdigit(*p) || *p == '-' || *p == '+') {
                    DC++;
                    while (*p && *p != ',' && *p != '\n') {
                        p++;
                    }
                } else {
                    p++;
                }
            }
        } else if (parsed.type == LINE_DIRECTIVE && strstr(line, ".string")) {
            const char *start = strchr(line, '"');
            if (start) {
                int len = 0;
                start++;
                while (*start && *start != '"') {
                    len++;
                    start++;
                }
                DC += len + 1; /* Include null terminator */
            }
        }
    }

    fclose(fp);
    return !has_errors();
}

/*
 * second_pass:
 * Re-parses the source file, resolves addresses, writes output files (.ob, .ent, .ext).
 */
bool second_pass(const char *filename) {
    FILE *fp;
    FILE *ob;
    FILE *ent;
    FILE *ext;
    char line[256];
    int line_num;
    int address;

    fp = fopen(filename, "r");
    if (!fp) {
        LOG_ERR("Cannot reopen file: %s", filename);
        return false;
    }

    ob = fopen("out.ob", "w");
    ent = fopen("out.ent", "w");
    ext = fopen("out.ext", "w");

    line_num = 0;
    address = 100;

    while (fgets(line, sizeof(line), fp)) {
        ParsedLine parsed;

        line_num++;
        if (!parse_line(line, line_num, &parsed)) {
            continue;
        }

        if (parsed.type == LINE_COMMAND) {
            int i;
            fprintf(ob, "%04d %s\n", address++, "<binary>");

            /* Write operands if any */
            for (i = 0; i < parsed.operand_count; i++) {
                Operand *op = &parsed.operands[i];
                if (op->type == OPERAND_DIRECT || op->type == OPERAND_RELATIVE) {
                    Symbol *sym = find_symbol(op->value);
                    if (sym) {
                        fprintf(ob, "%04d %s\n", address++, "<sym_bin>");
                        if (sym->type == SYMBOL_EXTERN) {
                            fprintf(ext, "%s %04d\n", sym->name, address - 1);
                        }
                    }
                } else {
                    fprintf(ob, "%04d %s\n", address++, "<op_bin>");
                }
            }
        } else if (parsed.type == LINE_DIRECTIVE && strstr(line, ".entry")) {
            char entry_label[MAX_SYMBOL_NAME];
            Symbol *sym;
            sscanf(line, ".entry %s", entry_label);
            mark_entry(entry_label);
            sym = find_symbol(entry_label);
            if (sym) {
                fprintf(ent, "%s %04d\n", sym->name, sym->address);
            }
        }
    }

    fclose(fp);
    fclose(ob);
    fclose(ent);
    fclose(ext);

    return true;
}

