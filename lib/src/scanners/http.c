#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "scanners/http.h"

// 5Mb.
const int MAX_BUFFER_SIZE = 1024 * 1024 * 5;
const int CHUNK_SIZE = 1024 * 512 * 1;

// Connect to the HTTP server.
int http_connect(Task task) {
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(80);
  server_addr.sin_addr.s_addr = inet_addr(task.address);

  int err = connect(task.socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (err == -1) {
    // Ignoring connection error logging.
    return -1;
  }

  return 0;
}

// Send a GET request to the HTTP server.
char* http_scan(Task task) {
  // Buffer used in the request and response.
  char request[64];
  snprintf(request, sizeof(request), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
           task.address);

  int err = send(task.socket_fd, request, strlen(request), 0);
  if (err == -1) {
    ERROR("GET", "Sending '%s'", task.address);
    return NULL;
  }

  char status[17];

  int n = recv(task.socket_fd, status, 17, 0);
  if (n == -1) {
    // Ignoring timeout error logging.
    return NULL;
  }

  // Check if the status code is 2xx.
  if (status[9] != '2') {
    return NULL;
  }

  char* buffer = malloc(CHUNK_SIZE);
  int current_size = 0;
  int buffer_size = CHUNK_SIZE;

  while (1) {
    // Check if we need to realloc before reading more data.
    if (current_size + CHUNK_SIZE > buffer_size) {
      buffer_size *= 2;
      if (buffer_size > MAX_BUFFER_SIZE) {
        buffer_size = MAX_BUFFER_SIZE;
      }

      char* temp = realloc(buffer, buffer_size);
      if (temp == NULL) {
        ERROR("GET", "Realloc of size %d failed", buffer_size);
        buffer = temp;
        break;
      }

      buffer = temp;
    }

    int n = recv(task.socket_fd, buffer + current_size, buffer_size - current_size, 0);
    if (n <= 0) {
      break;
    }

    current_size += n;
  }

  buffer[current_size] = '\0';

  INFO("GET", "Success '%s'", task.address);
  return buffer;
}
