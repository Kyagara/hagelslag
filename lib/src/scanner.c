#include "scanner.h"
#include "logger.h"

const char* get_scanner_name(void) {
  if (SCANNER == HTTP) {
    return "HTTP";
  } else if (SCANNER == MINECRAFT) {
    return "Minecraft";
  }
}

Scanner set_scanner(void) {
  Scanner scanner;

#if SCANNER == HTTP
  scanner.connect = http_connect;
  scanner.scan = http_scan;
#elif SCANNER == MINECRAFT
  scanner.connect = minecraft_connect;
  scanner.scan = minecraft_scan;
#else
#error "Unknown SCANNER type"
#endif

  INFO("MAIN", "Using %s scanner", get_scanner_name());
  return scanner;
}
