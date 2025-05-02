#include "logger.h"
#include <time.h>
#include <stdarg.h>

/* Current minimum log level to display */
static LogLevel current_log_level = LOG_DEBUG;

/*
 * level_to_str:
 * Converts a log level enum to a short string label.
 */
static const char *level_to_str(LogLevel level) {
    switch (level) {
        case LOG_DEBUG:
            return "DBG";
        case LOG_INF:
            return "INF";
        case LOG_WARN:
            return "WRN";
        case log_errOR:
            return "ERR";
        default:
            return "UNK";
    }
}

/*
 * asm_log_set_level:
 * Sets the current logging level.
 */
void asm_log_set_level(LogLevel level) {
    current_log_level = level;
}

/*
 * log_internal:
 * Generic logger for normal messages with timestamp and level.
 */
void log_internal(LogLevel level, const char *fmt, ...) {
    time_t t;
    struct tm *tm_info;
    char time_buf[9];
    va_list args;

    if (level < current_log_level) {
        return;
    }

    t = time(NULL);
    tm_info = localtime(&t);
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);

    printf("[%s] %s | ", time_buf, level_to_str(level));

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}

/*
 * log_asm_internal:
 * Special logger for assembler warnings and errors tied to specific file positions.
 */
void log_asm_internal(LogLevel level, const char *file, int line, int col, const char *fmt, ...) {
    const char *bold;
    const char *reset;
    const char *red_bold;
    const char *yellow_bold;
    const char *label_color;
    const char *label;
    va_list args;

    if (level < current_log_level) {
        return;
    }

    bold = "\033[1m";
    reset = "\033[0m";
    red_bold = "\033[1;31m";
    yellow_bold = "\033[1;33m";

    if (level == LOG_WARN) {
        label = "warning";
        label_color = yellow_bold;
    } else {
        label = "error";
        label_color = red_bold;
    }

    fprintf(stderr, "%s%s:%d:%d:%s ", bold, file, line, col, reset);
    fprintf(stderr, "%s%s:%s ", label_color, label, reset);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
    fflush(stderr);
}

/* Simple wrappers for different log levels */

void log_dbg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_internal(LOG_DEBUG, fmt, args);
    va_end(args);
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_internal(LOG_INF, fmt, args);
    va_end(args);
}

void log_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_internal(LOG_WARN, fmt, args);
    va_end(args);
}

void log_err(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_internal(log_errOR, fmt, args);
    va_end(args);
}

void asm_warn(const char *file, int line, int col, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_asm_internal(LOG_WARN, file, line, col, fmt, args);
    va_end(args);
}

void asm_err(const char *file, int line, int col, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_asm_internal(log_errOR, file, line, col, fmt, args);
    va_end(args);
}
