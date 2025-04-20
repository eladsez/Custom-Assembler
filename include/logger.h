#ifndef ASM_LOGGER_H
#define ASM_LOGGER_H

#include <stdio.h>
#include <stdarg.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

void asm_log_set_level(LogLevel level);

void log_internal(LogLevel level, const char *fmt, ...);
void log_asm_internal(LogLevel level, const char *file, int line, int col, const char *fmt, ...);

#define LOG_DBG(fmt, ...)  log_internal(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) log_internal(LOG_INFO,  fmt, ##__VA_ARGS__)
#define LOG_WRN(fmt, ...)  log_internal(LOG_WARN,  fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)  log_internal(LOG_ERROR, fmt, ##__VA_ARGS__)

#define ASM_WRN(file, line, col, fmt, ...)  log_asm_internal(LOG_WARN,  file , line , col, fmt, ##__VA_ARGS__)
#define ASM_ERR(file, line, col, fmt, ...)  log_asm_internal(LOG_ERROR, file, line , col, fmt, ##__VA_ARGS__)

#endif // ASM_LOGGER_H
