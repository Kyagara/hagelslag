#include "scanner.h"

Scanner get_scanner() {
  Scanner scanner;

  if (SCANNER == HTTP) {
    scanner.connect = http_connect;
    scanner.scan = http_scan;
    scanner.save = http_save;
  }

  return scanner;
}
