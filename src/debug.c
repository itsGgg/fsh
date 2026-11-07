#include "../include/debug.h"
#include "../include/utils.h"

void log_message(log_level_t level, const char *msg, const char *file, int line) {
    if (LOG_TRIGGER == LOG_UNIQ) {
        if (level != LOG_LEVEL)
            return;
    } else // LOG_TRIGGER == LOG_MAX
    {
        if (level > LOG_LEVEL)
            return;
    }

    const char *level_str;
    char buffer[512]; // Tampon pour le message
    size_t len;

    switch (level) {
    case LOG_INFO:
        level_str = "INFO";
        break;
    case LOG_WARNING:
        level_str = "WARNING";
        break;
    case LOG_ERROR:
        level_str = "ERROR";
        break;
    default:
        level_str = "UNKNOWN";
        break;
    }

    len = snprintf(buffer, sizeof(buffer), "[%s] %s:%d: %s\n", level_str, file, line, msg);

    // Si ca dépasse...
    if (len >= sizeof(buffer)) {
        const char *truncate_msg = "[LOG TRUNCATED]\n";
        write_err_no_ln(truncate_msg);
    } else {
        write_err_no_ln(buffer);
    }
}