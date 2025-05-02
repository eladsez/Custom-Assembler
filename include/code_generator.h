#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <stdio.h>
#include <stdbool.h>
#include "symbol_table.h"
#include "parser.h"

typedef struct {
    int address;
    unsigned short value;
} MemoryWord;


int encode_instruction(const ParsedLine *pline, int ic, unsigned short *out_words);
int encode_operand_word(const Operand *op, int curr_ic, unsigned short *word_out);
int encode_registers_word(const Operand *src, const Operand *dst, unsigned short *word_out);
int add_machine_word(int address, unsigned short value);

#endif