#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "parser.h"
#include "utils.h"


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
InstructionType lookup_instruction(const char *token) {
    size_t i;
    for (i = 0; i < sizeof(instructions) / sizeof(instructions[0]); i++) {
        if (strcmp(token, instructions[i].name) == 0) {
            return instructions[i].type;
        }
    }
    return INST_NONE;
}

Operand parse_operand(const char *str) {
    Operand op;
    size_t i;
    size_t len;

    op.type = OPERAND_NONE;
    strncpy(op.value, str, LABEL_LENGTH);
    op.value[LABEL_LENGTH] = '\0';

    if (str[0] == '#') {
        if (str[1] == '-' || str[1] == '+' || isdigit(str[1])) {
            op.type = OPERAND_IMMEDIATE;
        }
    } else if (str[0] == '&') {
        if (isalpha(str[1])) {
            len = strlen(str);
            for (i = 1; i < len; i++) {
                if (!isalnum(str[i])) return op;
            }
            op.type = OPERAND_RELATIVE;
        }
    } else if (is_register(str)) {
        op.type = OPERAND_REGISTER_DIRECT;
    } else if (isalpha(str[0])) {
        len = strlen(str);
        for (i = 0; i < len; i++) {
            if (!isalnum(str[i])) return op;
        }
        op.type = OPERAND_DIRECT;
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
    size_t i;

    strncpy(line_copy, line_input, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    strncpy(result->original_line, line_input, LINE_LENGTH + 1);
    result->original_line[LINE_LENGTH + 1] = '\0';

    line = trim_whitespace(line_copy);

   
    result->line_number = line_number;
    result->instruction = INST_NONE;
    result->operand_count = 0;
    result->label[0] = '\0';
    result->type = LINE_INVALID;
    result->err_msg[0] = '\0';
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
    if (!token) {
        snprintf(result->err_msg, MAX_MSG, "Empty token after stripping whitespace.");
        return false;
    }

    colon = strchr(token, ':');
    if (colon) {
        *colon = '\0';
        if (strlen(token) > LABEL_LENGTH || !isalpha(token[0])) {
            snprintf(result->err_msg, MAX_MSG, "Invalid label: '%s'", token);
            return false;
        }
        for (i = 0; i < strlen(token); i++) {
            if (!isalnum(token[i])) {
                snprintf(result->err_msg, MAX_MSG, "Invalid character in label: '%c'", token[i]);
                return false;
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
        if (strcmp(token, ".data") == 0 || strcmp(token, ".string") == 0) {
            result->type = LINE_DIRECTIVE;
            return true;
        } else if (strcmp(token, ".extern") == 0 || strcmp(token, ".entry") == 0) {
            result->type = LINE_DIRECTIVE;

            token = strtok(NULL, " \t");
            if (!token || strlen(token) > LABEL_LENGTH || !isalpha(token[0])) {
                snprintf(result->err_msg, MAX_MSG, "Invalid or missing label after directive.");
                result->type = LINE_INVALID;
                return false;
            }
            for (i = 0; i < strlen(token); i++) {
                if (!isalnum(token[i])) {
                    snprintf(result->err_msg, MAX_MSG, "Invalid character in directive label: '%c'", token[i]);
                    result->type = LINE_INVALID;
                    return false;
                }
            }
            strcpy(result->label, token);
            return true;
        } else {
            snprintf(result->err_msg, MAX_MSG, "Unknown directive '%s'", token);
            result->type = LINE_INVALID;
            return false;
        }
    }

    result->instruction = lookup_instruction(token);
    if (result->instruction == INST_NONE) {
        snprintf(result->err_msg, MAX_MSG, "Unknown instruction '%s'", token);
        result->type = LINE_INVALID;
        return false;
    }

    result->type = LINE_COMMAND;
    op_str = strtok(NULL, "");
    if (!op_str) {
        if (OP_PER_INST(result->instruction) != 0) {
            snprintf(result->err_msg, MAX_MSG, "Missing operand(s) for instruction '%s'", token);
            result->type = LINE_INVALID;
            return false;
        }
        return true;
    }

    op_str = trim_whitespace(op_str);

    if (OP_PER_INST(result->instruction) == 1) {
        first = trim_whitespace(op_str);
        result->operands[0] = parse_operand(first);
        result->operand_count = 1;

        if (result->operands[0].type == OPERAND_NONE) {
            snprintf(result->err_msg, MAX_MSG, "Invalid operand: '%s'", first);
            result->type = LINE_INVALID;
            return false;
        }

    } else if (OP_PER_INST(result->instruction) == 2) {
        char *comma = strchr(op_str, ',');
        if (!comma) {
            snprintf(result->err_msg, MAX_MSG, "Missing comma between operands.");
            result->type = LINE_INVALID;
            return false;
        }

        *comma = '\0';
        first = trim_whitespace(op_str);
        second = trim_whitespace(comma + 1);

        result->operands[0] = parse_operand(first);
        result->operands[1] = parse_operand(second);
        result->operand_count = 2;

        if (result->operands[0].type == OPERAND_NONE) {
            snprintf(result->err_msg, MAX_MSG, "Invalid first operand: '%s'", first);
            result->type = LINE_INVALID;
            return false;
        }

        if (result->operands[1].type == OPERAND_NONE) {
            snprintf(result->err_msg, MAX_MSG, "Invalid second operand: '%s'", second);
            result->type = LINE_INVALID;
            return false;
        }

        if (strchr(second, ',')) {
            snprintf(result->err_msg, MAX_MSG, "Too many operands.");
            result->type = LINE_INVALID;
            return false;
        }

    } else {
        snprintf(result->err_msg, MAX_MSG, "Instruction '%s' does not expect operands.", token);
        result->type = LINE_INVALID;
        return false;
    }

    return true;
}