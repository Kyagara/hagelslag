#ifndef HTTP_H
#define HTTP_H

#include <sqlite3.h>

void http_scan(sqlite3* db, sqlite3_stmt* insert_stmt, int socket_fd, const char* address);

#endif // HTTP_H
