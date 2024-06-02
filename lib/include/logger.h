int log_level_from_env();

void DEBUG(const char* event, const char* format, ...);
void INFO(const char* event, const char* format, ...);
void ERROR(const char* event, const char* format, ...);
void FATAL(const char* event, const char* format, ...);
