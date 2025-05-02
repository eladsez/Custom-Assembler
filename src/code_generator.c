

#include "code_generator.h"

MemoryWord *machine_code = NULL;
int machine_code_size = 0;
int machine_code_capacity = 0;

unsigned short encode_addressing_mode(OperandType type) {
    if (type == OPERAND_IMMEDIATE) return 0;
    if (type == OPERAND_DIRECT) return 1;
    if (type == OPERAND_RELATIVE) return 2;
    if (type == OPERAND_REGISTER_DIRECT) return 3;
    return 0;
}

unsigned short encode_register(const char *val) {
    if (strcmp(val, "r0") == 0) return 0;
    if (strcmp(val, "r1") == 0) return 1;
    if (strcmp(val, "r2") == 0) return 2;
    if (strcmp(val, "r3") == 0) return 3;
    if (strcmp(val, "r4") == 0) return 4;
    if (strcmp(val, "r5") == 0) return 5;
    if (strcmp(val, "r6") == 0) return 6;
    if (strcmp(val, "r7") == 0) return 7;
    return 0xFFFF; /* Invalid register */
}

int encode_instruction(const ParsedLine *pline, int ic, unsigned short *out_words) {
    unsigned short word = 0;
    unsigned short src_mode = 0;
    unsigned short dst_mode = 0;
    unsigned short opcode = 0;

    if (pline->operand_count == 2) {
        src_mode = encode_addressing_mode(pline->operands[0].type);
        dst_mode = encode_addressing_mode(pline->operands[1].type);
    } else if (pline->operand_count == 1) {
        dst_mode = encode_addressing_mode(pline->operands[0].type);
    }

    opcode = (unsigned short)(pline->instruction & 0xF);

    word |= (0 << 12);
    word |= (dst_mode & 0x3) << 10;
    word |= (src_mode & 0x3) << 8;
    word |= (opcode & 0xF) << 4;

    out_words[0] = word;
    return 1;
}


int encode_operand_word(const Operand *op, int curr_ic, unsigned short *word_out) {
    unsigned short word = 0;
    Symbol *sym;
    long value;

    if (op->type == OPERAND_IMMEDIATE) {
        value = strtol(op->value + 1, NULL, 10);
        if (value < -8192 || value > 8191) return -1;  /* 14-bit signed range */
        if (value < 0) value = (1 << 14) + value;
        word = (unsigned short)value;
        word |= (0 << 12);  /* ARE = 00 */
        *word_out = word;
        return 1;
    }

    if (op->type == OPERAND_RELATIVE) {
        sym = find_symbol(op->value + 1);
        if (!sym) return -1;
        value = sym->address - curr_ic;
        if (value < -8192 || value > 8191) return -1;
        if (value < 0) value = (1 << 14) + value;
        word = (unsigned short)value;
        word |= (2 << 12);  /* ARE = 10 */
        *word_out = word;
        return 1;
    }

    if (op->type == OPERAND_DIRECT) {
        sym = find_symbol(op->value);
        if (!sym) return -1;
        word = (unsigned short)(sym->address & 0x0FFF);
        word |= (sym->type == SYMBOL_EXTERN) ? (1 << 12) : (2 << 12);
        *word_out = word;
        return 1;
    }

    return 0;
}


int add_machine_word(int address, unsigned short value) {
    MemoryWord *temp;

    if (machine_code_size >= machine_code_capacity) {
        int new_capacity = (machine_code_capacity == 0) ? 64 : machine_code_capacity * 2;
        temp = realloc(machine_code, new_capacity * sizeof(MemoryWord));
        if (!temp) return 0;
        machine_code = temp;
        machine_code_capacity = new_capacity;
    }

    machine_code[machine_code_size].address = address;
    machine_code[machine_code_size].value = value;
    machine_code_size++;
    return 1;
}