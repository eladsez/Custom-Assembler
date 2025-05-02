#include "symbol_table.h"

Symbol *symbol_table = NULL;

void free_symbol_table() {
    Symbol *current = symbol_table;
    while (current != NULL) {
        Symbol *next = current->next;
        free(current);
        current = next;
    }
    symbol_table = NULL;
}

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