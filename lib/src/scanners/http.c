#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include <sqlite3.h>

#include "ip.h"
#include "logger.h"
#include "scanners/http.h"

// Connect to the HTTP server.
int http_connect(int socket_fd, const char* address) {
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(80);

  // Converting the IP address.
  int err = inet_pton(AF_INET, address, &server_addr.sin_addr);
  if (err <= 0) {
    ERROR("CONN", "Converting '%s'", address);
    return -1;
  }

  err = connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (err == -1) {
    return -1;
  }

  return 0;
}

// Connect to the HTTP server and get a response.
int http_scan(int socket_fd, const char* address, Result* result) {
  // Send a GET request to the HTTP server.

  // Buffer used in the request and response.
  char buffer[64];

  snprintf(buffer, sizeof(buffer), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
           address);

  int err = send(socket_fd, buffer, strlen(buffer), 0);
  if (err == -1) {
    ERROR("GET", "Sending '%s'", address);
    return -1;
  }

  // Only getting a response is good enough,
  // no need to read the buffer or have a big buffer.
  ssize_t n = recv(socket_fd, buffer, sizeof(buffer), 0);
  if (n == -1) {
    return -1;
  }

  // Check if the status code is 2xx.
  if (n < 16 || buffer[9] != '2') {
    return -1;
  }

  result->address = address;
  INFO("GET", "Success '%s'", address);
  return 0;
}

// Insert the address into the database as an integer.
void http_save(sqlite3* db, sqlite3_stmt* insert_stmt, const char* address) {
  int ip = address_to_int(address);

  sqlite3_bind_int(insert_stmt, 1, ip);

  int err = sqlite3_step(insert_stmt);
  if (err != SQLITE_DONE && err != SQLITE_BUSY && err != SQLITE_LOCKED) {
    ERROR("DATABASE", "Can't save '%s' in table: %s", address, sqlite3_errmsg(db));
  }

  sqlite3_reset(insert_stmt);
  sqlite3_clear_bindings(insert_stmt);
}
