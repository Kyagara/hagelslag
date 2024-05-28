#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// -1 = trace, 0 = debug, 1 = info, 2 = error.
int log_level = -1;

void log_formatted(const char *level, const char *event, const char *msg,
                   va_list args);

void trace(const char *event, const char *msg, ...) {
  if (log_level <= -1) {
    va_list args;
    va_start(args, msg);
    log_formatted("TRACE", event, msg, args);
    va_end(args);
  }
}

void debug(const char *event, const char *msg, ...) {
  if (log_level <= 0) {
    va_list args;
    va_start(args, msg);
    log_formatted("DEBUG", event, msg, args);
    va_end(args);
  }
}

void error(const char *event, const char *msg, ...) {
  if (log_level <= 1) {
    va_list args;
    va_start(args, msg);
    log_formatted("ERROR", event, msg, args);
    va_end(args);
  }
}

void info(const char *event, const char *msg, ...) {
  if (log_level <= 2) {
    va_list args;
    va_start(args, msg);
    log_formatted("INFO", event, msg, args);
    va_end(args);
  }
}

// Set the log level from the environment variable LOG_LEVEL
void log_level_from_env() {
  char *value = getenv("LOGLEVEL");
  if (value != NULL) {
    log_level = atoi(value);
  }

  info("LOG", "LEVEL: %d", log_level);
}

// Print the formatted message string.
//
// [LEVEL] EVENT MESSAGE
void log_formatted(const char *level, const char *event, const char *format,
                   va_list args) {

  if (level == NULL || event == NULL || format == NULL) {
    fprintf(stderr, "NULL level, event, or format\n");
    return;
  }

  char buffer[128];
  snprintf(buffer, sizeof(buffer), "[%s] %s %s\n", level, event, format);

  vfprintf(stderr, buffer, args);
}
