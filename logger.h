int log_level_from_env();

void debug(const char *event, const char *format, ...);
void info(const char *event, const char *format, ...);
void error(const char *event, const char *format, ...);
void fatal(const char *event, const char *format, ...);
