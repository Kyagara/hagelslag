void log_level_from_env();

void trace(const char *event, const char *format, ...);
void debug(const char *event, const char *format, ...);
void info(const char *event, const char *format, ...);
void error(const char *event, const char *format, ...);
