#include "scanner.h"

Scanner set_scanner() {
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

  return scanner;
}

const char* get_scanner_name() {
  if (SCANNER == HTTP) {
    return "HTTP";
  } else if (SCANNER == MINECRAFT) {
    return "Minecraft";
  }
}
