#include <stdio.h>
#include <string.h>
#include <ctype.h>  
#include <stdlib.h>
#include "directive_handler.h"
#include "utils.h"

DirectiveType get_directive_type(const char *word) {
    if (word == NULL) {
        return DIRECTIVE_NONE;
    }

    if (strcmp(word, "data") == 0) {
        return DIRECTIVE_DATA;
    } else if (strcmp(word, "string") == 0) {
        return DIRECTIVE_STRING;
    } else if (strcmp(word, "entry") == 0) {
        return DIRECTIVE_ENTRY;
    } else if (strcmp(word, "extern") == 0) {
        return DIRECTIVE_EXTERN;
    }

    return DIRECTIVE_NONE;
}

/* Helper to check if a string is a valid signed number */
static int is_valid_number(const char *s) {
    if (*s == '+' || *s == '-') s++;
    if (!*s) return 0; /* sign with no digits */
    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

void handle_data_directive(const char *content) {
    char buffer[1024];
    int i = 0, len, has_number = 0;
    const char *p;
    char token[64];
    int token_i = 0;

    if (!content) {
        printf("Error: .data directive is empty.\n");
        return;
    }

    strncpy(buffer, content, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    p = trim_whitespace(buffer);
    len = strlen(p);

    if (len == 0) {
        printf("Error: .data directive must contain at least one number.\n");
        return;
    }

    if (p[0] == ',' || p[len - 1] == ',') {
        printf("Error: .data cannot start or end with a comma.\n");
        return;
    }

    for (i = 0; i <= len; i++) {
        if (p[i] == ',' || p[i] == '\0') {
            token[token_i] = '\0';
            if (token_i == 0) {
                printf("Error: .data has empty value between commas.\n");
                return;
            }
            if (!is_valid_number(token)) {
                printf("Error: Invalid number '%s' in .data directive.\n", token);
                return;
            }
            printf("Valid number: %s\n", token);
            has_number = 1;
            token_i = 0; /* Reset for next token */
        } else {
            if (token_i < (int)(sizeof(token) - 1)) {
                token[token_i++] = p[i];
            }
        }
    }

    if (!has_number) {
        printf("Error: .data directive must contain at least one number.\n");
    }
}


void handle_string_directive(const char *content) {
    /* Placeholder for .string directive logic */
    printf("Handling .string with content: %s\n", content);
}

void handle_entry_directive(const char *content) {
    /* Placeholder for .entry directive logic */
    printf("Handling .entry with content: %s\n", content);
}

void handle_extern_directive(const char *content) {
    /* Placeholder for .extern directive logic */
    printf("Handling .extern with content: %s\n", content);
}


