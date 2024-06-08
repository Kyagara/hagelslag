#ifndef HTTP_H
#define HTTP_H

#include <sqlite3.h>

typedef struct {
  const char* address;
} Result;

int http_connect(int socket_fd, const char* address);
int http_scan(int socket_fd, const char* address, Result* result);
void http_save(sqlite3* db, sqlite3_stmt* insert_stmt, const char* address);

#endif // HTTP_H
