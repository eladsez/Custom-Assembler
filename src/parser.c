#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "parser.h"


static char *trim_whitespace(char *str) {
    while (isspace(*str)) str++;
    if (*str == '\0') return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
    return str;
}


static bool is_register(const char *str) {
    return strlen(str) == 2 && str[0] == 'r' && str[1] >= '0' && str[1] <= '7';
}


typedef struct {
    const char *name;
    InstructionType type;
} InstructionMap;

static InstructionMap instructions[] = {
    {"mov", INST_MOV}, {"cmp", INST_CMP}, {"add", INST_ADD}, {"sub", INST_SUB}, {"lea", INST_LEA},
    {"clr", INST_CLR}, {"not", INST_NOT}, {"inc", INST_INC}, {"dec", INST_DEC},
    {"jmp", INST_JMP}, {"bne", INST_BNE}, {"jsr", INST_JSR},
    {"red", INST_RED}, {"prn", INST_PRN},
    {"rts", INST_RTS}, {"stop", INST_STOP}
};

static InstructionType lookup_instruction(const char *token) {
    size_t i;
    for (i = 0; i < sizeof(instructions) / sizeof(instructions[0]); i++) {
        if (strcmp(token, instructions[i].name) == 0) {
            return instructions[i].type;
        }
    }
    return INST_NONE;
}


static Operand parse_operand(const char *str) {
    Operand op = { .type = OPERAND_NONE };
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
    }
    else{
        /* error */
    }
    return op;
}

bool parse_line(const char *line_input, int line_number, ParsedLine *result) {
    char line_copy[256];
    strncpy(line_copy, line_input, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    char *line = trim_whitespace(line_copy);
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

    char *token = strtok(line, " \t");
    if (!token) return false;

    /* check for label */
    char *colon = strchr(token, ':');
    if (colon) {
        *colon = '\0';
        if (strlen(token) > MAX_LABEL_LENGTH){
            /* error: label is to long */
            return false;
        } 
        strcpy(result->label, token);
        token = strtok(NULL, " \t");
        if (!token) {
            result->type = LINE_LABEL_ONLY;
            return true;
        }
    }

    /* Directive */
    if (token[0] == '.') {
        result->type = LINE_DIRECTIVE;
        /* TODO, .data or .string – could parse more here */
        return true;
    }

    /* Instruction */
    InstructionType inst = lookup_instruction(token);
    if (inst == INST_NONE) {
        /* error: invalid instruction */
        result->type = LINE_INVALID;
        return false;
    }

    result->type = LINE_COMMAND;
    result->instruction = inst;

    /* Parse operands (if any) */
    char *op_str = strtok(NULL, "");
    if (!op_str) {
        result->operand_count = 0;
        return true;
    }

    op_str = trim_whitespace(op_str);
    char *first = strtok(op_str, ",");
    if (first) {
        first = trim_whitespace(first);
        result->operands[0] = parse_operand(first);
        result->operand_count = 1;

        char *second = strtok(NULL, ",");
        if (second) {
            second = trim_whitespace(second);
            result->operands[1] = parse_operand(second);
            result->operand_count = 2;

            /* Check for extra operands */
            if (strtok(NULL, ",")) {
                /* error: Invalid because more then two operands*/
                result->type = LINE_INVALID;
                return false;
            }
        }
    }

    return true;
}
