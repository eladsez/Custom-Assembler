#include "logger.h"
#include <time.h>
#include <stdarg.h>

static LogLevel current_log_level = LOG_DEBUG;

static const char *level_to_str(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DBG";
        case LOG_INFO:  return "INF";
        case LOG_WARN:  return "WRN";
        case LOG_ERROR: return "ERR";
        default:        return "UNK";
    }
}

void asm_log_set_level(LogLevel level) {
    current_log_level = level;
}

void log_internal(LogLevel level, const char *fmt, ...) {
    if (level < current_log_level) return;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_buf[9];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);

    printf("[%s] %s | ", time_buf, level_to_str(level));

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}

void log_asm_internal(LogLevel level, const char *file, int line, int col, const char *fmt, ...) {
    if (level < current_log_level) return;

    const char *bold      = "\033[1m";
    const char *reset     = "\033[0m";
    const char *red_bold  = "\033[1;31m";
    const char *yellow_bold = "\033[1;33m";

    const char *label_color;
    const char *label;

    if (level == LOG_WARN) {
        label = "warning";
        label_color = yellow_bold;
    } else {
        label = "error";
        label_color = red_bold;
    }

    fprintf(stderr, "%s%s:%d:%d:%s ", bold, file, line, col, reset);

    fprintf(stderr, "%s%s:%s ", label_color, label, reset);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
}
