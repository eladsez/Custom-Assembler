#ifndef DIRECTIVE_HANDLER_H
#define DIRECTIVE_HANDLER_H

typedef enum {
    DIRECTIVE_NONE,
    DIRECTIVE_DATA,
    DIRECTIVE_STRING,
    DIRECTIVE_ENTRY,
    DIRECTIVE_EXTERN
} DirectiveType;

DirectiveType get_directive_type(const char *word);

/* Placeholder handlers for each directive */
void handle_data_directive(const char *content);
void handle_string_directive(const char *content);
void handle_entry_directive(const char *content);
void handle_extern_directive(const char *content);

#endif

