#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <sqlite3.h>

#include "logger.h"

uint32_t address_to_int(const char* address);

int conn(int socket_fd, const char* ip);
int get(int socket_fd, const char* ip);

// Creates a socket, sets a timeout of 1 second, returns -1 on error or socket_fd.
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

// Connect to the server and get a response.
void scan(sqlite3* db, sqlite3_stmt* insert_stmt, int socket_fd, const char* address) {
  int err = conn(socket_fd, address);
  if (err == -1) {
    return;
  }

  int ip = address_to_int(address);

  err = get(socket_fd, address);
  if (err == -1) {
    return;
  }

  // Get succeeded, insert into the database.
  sqlite3_bind_int(insert_stmt, 1, ip);

  err = sqlite3_step(insert_stmt);
  if (err != SQLITE_DONE && err != SQLITE_BUSY && err != SQLITE_LOCKED) {
    ERROR("DATABASE", "Can't save '%s' in table: %s", address, sqlite3_errmsg(db));
  }

  sqlite3_reset(insert_stmt);
  sqlite3_clear_bindings(insert_stmt);
}

// Connect to the server, return -1 on error.
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

  return connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
}

// Send a GET request, return -1 on error.
int get(int socket_fd, const char* ip) {
  // Buffer used in the request and response.
  char buffer[128];

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
