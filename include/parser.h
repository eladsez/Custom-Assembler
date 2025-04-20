#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

/* Max label length from doc */
#define MAX_LABEL_LENGTH 31
#define OP_PER_INST(ints_type) (ints_type < INST_CLR ? 2 : ints_type < INST_RTS ? 1 : 0)

/* Type of line parsed from source */
typedef enum {
    LINE_EMPTY,
    LINE_COMMENT,
    LINE_LABEL_ONLY,
    LINE_DIRECTIVE,
    LINE_COMMAND,
    LINE_INVALID
} LineType;

/* Instructions from the imaginary computer */
typedef enum {
    INST_NONE = -1,
    /* first instruction group */
    INST_MOV = 0,
    INST_CMP,
    INST_ADD,
    INST_SUB,
    INST_LEA,
    /* second */
    INST_CLR,
    INST_NOT,
    INST_INC,
    INST_DEC,
    INST_JMP,
    INST_BNE,
    INST_JSR,
    INST_RED,
    INST_PRN,
    /* third */
    INST_RTS,
    INST_STOP
} InstructionType;

/* Operand addressing types */
/*

*/
typedef enum {
    OPERAND_NONE = -1,
    OPERAND_IMMEDIATE, 
    OPERAND_DIRECT,           
    OPERAND_RELATIVE,         
    OPERAND_REGISTER_DIRECT   
} OperandType;

/* Structure for a single operand */
typedef struct {
    OperandType type;
    char value[MAX_LABEL_LENGTH + 1];
} Operand;

/* Parsed line result */
typedef struct {
    LineType type;
    int line_number;
    char label[MAX_LABEL_LENGTH + 1];  
    InstructionType instruction;
    Operand operands[2];               
    int operand_count;
} ParsedLine;

/**
 * Parses a single line of assembly code.
 *
 * @param line Raw line input from the file
 * @param line_number Line number (for error tracking)
 * @param result Pointer to ParsedLine struct to populate
 * @return true if parsing was successful, false if a syntax error was found
 */
bool parse_line(const char *line, int line_number, ParsedLine *result);

#endif
