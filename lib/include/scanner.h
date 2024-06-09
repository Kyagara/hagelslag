#ifndef SCANNER_H
#define SCANNER_H

typedef enum {
  HTTP,
  MINECRAFT,
} ScannerType;

#ifndef SCANNER
#define SCANNER HTTP
#endif

#define HTTP 1
#define MINECRAFT 2

#if SCANNER == HTTP
#include "scanners/http.h"
#elif SCANNER == MINECRAFT
#include "scanners/minecraft.h"
#else
#error "Unknown SCANNER type"
#endif

typedef int Connect(Task task);
typedef char* Scan(Task task);

typedef struct {
  Connect* connect;
  Scan* scan;
} Scanner;

Scanner set_scanner();
const char* get_scanner_name();

#endif // SCANNER_H
