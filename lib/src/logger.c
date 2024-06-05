#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// -1 = debug, 0 = info, 1 = error. Fatal will always exit no matter the level.
int log_level = -1;

// Set the log level using the environment variable LOG_LEVEL.
int log_level_from_env() {
  char* value = getenv("LOG_LEVEL");
  if (value != NULL) {
    log_level = atoi(value);
  }

  return log_level;
}

// Print the formatted message string to stderr.
//
// [LEVEL] EVENT FORMATTED_STRING
void log_formatted(const char* level, const char* event, const char* format, va_list args) {
  if (level == NULL || event == NULL || format == NULL) {
    fprintf(stderr, "NULL level, event or format\n");
    return;
  }

  time_t now = time(NULL);
  char time_buf[10];
  strftime(time_buf, sizeof(time_buf), "%H:%M:%S", localtime(&now));

  char log_buf[128];
  snprintf(log_buf, sizeof(log_buf), "%s %s %s - %s\n", level, time_buf, event, format);

  vfprintf(stderr, log_buf, args);
}

// cool <(= w =)>
#define LOG_FUNCTION(level_name, id)                                                               \
  void level_name(const char* event, const char* format, ...) {                                    \
    if (id != 2 && id < log_level) {                                                               \
      return;                                                                                      \
    }                                                                                              \
    va_list args;                                                                                  \
    va_start(args, format);                                                                        \
    log_formatted(#level_name, event, format, args);                                               \
    va_end(args);                                                                                  \
    if (id == 2) {                                                                                 \
      exit(1);                                                                                     \
    }                                                                                              \
  }

LOG_FUNCTION(DEBUG, -1)
LOG_FUNCTION(INFO, 0)
LOG_FUNCTION(ERROR, 1)
LOG_FUNCTION(FATAL, 2)
