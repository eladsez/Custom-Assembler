#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "parser.h"

/*
 * trim_whitespace:
 * Removes leading and trailing whitespace characters from a string.
 */
static char *trim_whitespace(char *str) {
    char *end;
    while (isspace(*str)) str++;
    if (*str == '\0') return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
    return str;
}

/*
 * is_register:
 * Checks if a string represents a valid register (r0 to r7).
 */
static bool is_register(const char *str) {
    return strlen(str) == 2 && str[0] == 'r' && str[1] >= '0' && str[1] <= '7';
}

/* Structure mapping instruction names to types */
typedef struct {
    const char *name;
    InstructionType type;
} InstructionMap;

/* List of supported instructions */
static InstructionMap instructions[] = {
    {"mov", INST_MOV}, {"cmp", INST_CMP}, {"add", INST_ADD}, {"sub", INST_SUB}, {"lea", INST_LEA},
    {"clr", INST_CLR}, {"not", INST_NOT}, {"inc", INST_INC}, {"dec", INST_DEC},
    {"jmp", INST_JMP}, {"bne", INST_BNE}, {"jsr", INST_JSR},
    {"red", INST_RED}, {"prn", INST_PRN},
    {"rts", INST_RTS}, {"stop", INST_STOP}
};

/*
 * lookup_instruction:
 * Finds an instruction type by its name.
 */
static InstructionType lookup_instruction(const char *token) {
    size_t i;
    for (i = 0; i < sizeof(instructions) / sizeof(instructions[0]); i++) {
        if (strcmp(token, instructions[i].name) == 0) {
            return instructions[i].type;
        }
    }
    return INST_NONE;
}

/*
 * parse_operand:
 * Parses a single operand string and determines its addressing type.
 */
static Operand parse_operand(const char *str) {
    Operand op;

    op.type = OPERAND_NONE;
    strncpy(op.value, str, MAX_LABEL_LENGTH);
    op.value[MAX_LABEL_LENGTH] = '\0';

    if (str[0] == '#') {
        op.type = OPERAND_IMMEDIATE;
    } else if (str[0] == '%') {
        op.type = OPERAND_RELATIVE;
    } else if (is_register(str)) {
        op.type = OPERAND_REGISTER_DIRECT;
    } else if (isalpha(str[0])) {
        op.type = OPERAND_DIRECT;
    } else {
        /* Unknown operand type */
    }
    return op;
}


/*
 * parse_line:
 * Parses a full line of assembly code into a ParsedLine structure.
 * Determines the line type, instruction type, and operands.
 */
bool parse_line(const char *line_input, int line_number, ParsedLine *result) {
    char line_copy[256];
    char *line;
    char *token;
    char *colon;
    char *op_str;
    char *first;
    char *second;

    strncpy(line_copy, line_input, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    line = trim_whitespace(line_copy);

    /* Initialize parsed line defaults */
    result->line_number = line_number;
    result->instruction = INST_NONE;
    result->operand_count = 0;
    result->label[0] = '\0';
    result->type = LINE_INVALID;
    result->operands[0].type = OPERAND_NONE;
    result->operands[1].type = OPERAND_NONE;

    if (*line == '\0') {
        result->type = LINE_EMPTY;
        return true;
    }

    if (*line == ';') {
        result->type = LINE_COMMENT;
        return true;
    }

    token = strtok(line, " \t");
    if (!token) return false;

    colon = strchr(token, ':');
    if (colon) {
        *colon = '\0';
        if (strlen(token) > MAX_LABEL_LENGTH) {
            return false;
        }
        if (!isalpha(token[0])) {
            return false;
        }
        {
            size_t i;
            for (i = 0; i < strlen(token); i++) {
                if (!isalnum(token[i])) {
                    return false;
                }
            }
        }
        strcpy(result->label, token);
        token = strtok(NULL, " \t");
        if (!token) {
            result->type = LINE_LABEL_ONLY;
            return true;
        }
    }

    if (token[0] == '.') {
/* TODO insert the handle functions */ 

       return true;         

    }
    /* Check for instruction */
    result->instruction = lookup_instruction(token);
    if (result->instruction == INST_NONE) {
        result->type = LINE_INVALID;
        return false;
    }

    result->type = LINE_COMMAND;
  /* Parse operands (if any) */
    op_str = strtok(NULL, "");
    if (!op_str) {
        result->operand_count = 0;
        return true;
    }

    op_str = trim_whitespace(op_str);
    first = strtok(op_str, ",");
    if (first) {
        first = trim_whitespace(first);
        result->operands[0] = parse_operand(first);
        result->operand_count = 1;

        second = strtok(NULL, ",");
        if (second) {
            second = trim_whitespace(second);
            result->operands[1] = parse_operand(second);
            result->operand_count = 2;

            if (strtok(NULL, ",")) {
                result->type = LINE_INVALID;
                return false;
            }
        }
    }

    return true;
}

