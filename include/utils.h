
#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

char *trim_whitespace(char *str);
bool is_register(const char *str);
char *strdup_c90(const char *src);

#endif