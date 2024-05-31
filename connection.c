#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "file.h"
#include "logger.h"

int conn(int socketfd, char *ip);
int get(int socketfd, char *ip);

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

void try_connection(int socket_fd, char *ip) {
  int err = conn(socket_fd, ip);
  if (err == -1) {
    return;
  }

  save_connection(ip);

  err = get(socket_fd, ip);
  if (err == -1) {
    return;
  }

  save_get(ip);
}

// Connect to the server, returns 0 on success, -1 on IP conversion error, -2
// on connection error.
int conn(int socket_fd, char *ip) {
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
  err = connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
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
int get(int socket_fd, char *ip) {
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
    ERROR("GET", "Receiving '%s'", ip);
    return -1;
  }

  INFO("GET", "Success '%s'", ip);
  return 0;
}
