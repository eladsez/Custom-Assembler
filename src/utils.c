
#include "utils.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*
 * trim_whitespace:
 * Removes leading and trailing whitespace characters from a string.
 */
char *trim_whitespace(char *str) {
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
bool is_register(const char *str) {
    return strlen(str) == 2 && str[0] == 'r' && str[1] >= '0' && str[1] <= '7';
}

char *strdup_c90(const char *src) {
    char *copy;
    size_t len = strlen(src) + 1;
    copy = (char *)malloc(len);
    if (copy != NULL) {
        strcpy(copy, src);
    }
    return copy;
}