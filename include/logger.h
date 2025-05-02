#ifndef ASM_LOGGER_H
#define ASM_LOGGER_H

#include <stdio.h>
#include <stdarg.h>

typedef enum {
    LOG_DEBUG,
    LOG_INF,
    LOG_WARN,
    log_errOR
} LogLevel;

void asm_log_set_level(LogLevel level);

void log_internal(LogLevel level, const char *fmt, ...);
void log_asm_internal(LogLevel level, const char *file, int line, int col, const char *fmt, va_list args);

void log_dbg(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_err(const char *fmt, ...);

void asm_warn(const char *file, int line, int col, const char *fmt, ...);
void asm_err(const char *file, int line, int col, const char *fmt, ...);

#endif /* ASM_LOGGER_H */
