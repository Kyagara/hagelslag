#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "logger.h"

// Creates a socket, sets the timeout, returns the socketfd on success, -1 on
// error.
int create_socket() {
  int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd == -1) {
    error("SOCKET", "CREATE");
    return -1;
  }

  info("SOCKET", "OPEN");

  struct timeval timeval;
  timeval.tv_sec = 1;
  timeval.tv_usec = 0;
  setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, &timeval, sizeof(timeval));

  info("SOCKET", "TIMEOUT %ds", timeval.tv_sec);
  return socketfd;
}

// Connect to the server, returns 0 on success, -1 on IP conversion error, -2
// on connection error.
int conn(int socketfd, char *ip) {
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(80);

  // Converting the IP address
  int err = inet_pton(AF_INET, ip, &server_addr.sin_addr);
  if (err <= 0) {
    error("CONN", "CONV %s", ip);
    return -1;
  }

  // Connecting to the server
  err = connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
    error("CONN", "CONNECT %s", ip);
    return -1;
  }

  debug("CONN", "SUCCESS %s", ip);
  return 0;
}

// Send a GET request, returns 0 on success, -1 on send error, -2 on recv
// error.
int get(int socketfd, char *ip) {
  // Buffer used in the request and response
  char buffer[100];

  snprintf(buffer, sizeof(buffer),
           "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", ip);

  int err = send(socketfd, buffer, strlen(buffer), 0);
  if (err == -1) {
    error("GET", "SEND %s", ip);
    return -1;
  }

  // Only getting a response is good enough,
  // no need to read the buffer or have a big buffer
  ssize_t n = recv(socketfd, buffer, sizeof(buffer), 0);
  if (n == -1) {
    error("GET", "RECV %s", ip);
    return -1;
  }

  debug("GET", "SUCCESS %s", ip);
  return 0;
}
