
#include "utils.h"
#include <ctype.h>
#include <string.h>

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