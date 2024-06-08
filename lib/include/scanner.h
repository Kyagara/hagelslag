#ifndef SCANNER_H
#define SCANNER_H

#include <sqlite3.h>

typedef enum {
  HTTP,
} ScannerType;

#ifndef SCANNER
#define SCANNER HTTP
#endif

#if SCANNER == HTTP
#include "scanners/http.h"
#endif

typedef int Connect(int socket_fd, const char* address);
typedef int Scan(int socket_fd, const char* address, Result* result);
typedef void Save(sqlite3* db, sqlite3_stmt* insert_stmt, const char* address);

typedef struct {
  Connect* connect;
  Scan* scan;
  Save* save;
} Scanner;

Scanner get_scanner();

#endif // SCANNER_H
