#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <sqlite3.h>

#include "logger.h"

uint32_t address_to_int(const char* address);

int conn(int socket_fd, const char* ip);
int get(int socket_fd, const char* ip);

// Creates a socket, sets the timeout, returns the socketfd on success, -1 on
// error.
int create_socket() {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    ERROR("SOCKET", "Creating socket");
    return -1;
  }

  struct timeval timeval;
  timeval.tv_sec = 1;
  timeval.tv_usec = 0;

  setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeval, sizeof(timeval));
  setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeval, sizeof(timeval));

  return socket_fd;
}

// Tries to connect to the server and get a response.
// If a connect is successful, it will be saved in the database in 'connection'.
// If a GET is successful, it will be saved in the database in 'get' instead.
void scan(sqlite3* db, sqlite3_stmt* conn_stmt, sqlite3_stmt* get_stmt, int socket_fd,
          const char* address) {

  int err = conn(socket_fd, address);
  if (err == -1) {
    return;
  }

  int ip = address_to_int(address);

  err = get(socket_fd, address);
  if (err == -1) {
    // Get failed, only save in 'connection' table.
    sqlite3_bind_int(conn_stmt, 1, ip);

    err = sqlite3_step(conn_stmt);
    if (err != SQLITE_DONE && err != SQLITE_BUSY && err != SQLITE_LOCKED) {
      ERROR("DATABASE", "Can't save '%s' in 'connection' table: %s", address, sqlite3_errmsg(db));
    }

    sqlite3_reset(conn_stmt);
    sqlite3_clear_bindings(conn_stmt);
    return;
  }

  // Get succeeded, only save in 'get' table.
  sqlite3_bind_int(get_stmt, 1, ip);

  err = sqlite3_step(get_stmt);
  if (err != SQLITE_DONE && err != SQLITE_BUSY && err != SQLITE_LOCKED) {
    ERROR("DATABASE", "Can't save '%s' in 'get' table: %s", address, sqlite3_errmsg(db));
  }

  sqlite3_reset(get_stmt);
  sqlite3_clear_bindings(get_stmt);
}

// Connect to the server, returns 0 on success, -1 on IP conversion error, -2
// on connection error.
int conn(int socket_fd, const char* ip) {
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(80);

  // Converting the IP address.
  int err = inet_pton(AF_INET, ip, &server_addr.sin_addr);
  if (err <= 0) {
    ERROR("CONN", "Converting '%s'", ip);
    return -1;
  }

  // Connecting to the server.
  err = connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (err == -1) {
    // Not logging this error to prevent spamming on debug level.
    // Don't think its necessary to log that there was an error connecting.
    return -1;
  }

  INFO("CONN", "Success '%s'", ip);
  return 0;
}

// Send a GET request, returns 0 on success, -1 on send error, -2 on recv
// error.
int get(int socket_fd, const char* ip) {
  // Buffer used in the request and response.
  char buffer[100];

  snprintf(buffer, sizeof(buffer), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", ip);

  int err = send(socket_fd, buffer, strlen(buffer), 0);
  if (err == -1) {
    ERROR("GET", "Sending '%s'", ip);
    return -1;
  }

  // Only getting a response is good enough,
  // no need to read the buffer or have a big buffer.
  ssize_t n = recv(socket_fd, buffer, sizeof(buffer), 0);
  if (n == -1) {
    // Not logging this error to prevent spamming on debug level.
    // Don't think its necessary to log that there was an error (most probably a timeout) receiving
    // a response.
    return -1;
  }

  INFO("GET", "Success '%s'", ip);
  return 0;
}

uint32_t address_to_int(const char* address) {
  struct in_addr addr;
  inet_pton(AF_INET, address, &addr);
  return ntohl(addr.s_addr);
}
