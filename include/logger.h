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

void LOG_DBG(const char *fmt, ...);
void LOG_INFO(const char *fmt, ...);
void LOG_WRN(const char *fmt, ...);
void LOG_ERR(const char *fmt, ...);

void ASM_WRN(const char *file, int line, int col, const char *fmt, ...);
void ASM_ERR(const char *file, int line, int col, const char *fmt, ...);

#endif /* ASM_LOGGER_H */
