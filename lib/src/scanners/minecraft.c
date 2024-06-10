#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "scanners/minecraft.h"

int handshake(const char* address, char* buffer);
int read_byte(const int socket_fd, char* buffer);
int read_varint(const int socket_fd);

const int MAX_LENGTH = 33000;
const int MAX_JSON_LENGTH = 32770;

int port = 25565;
unsigned char port_high = 255;
unsigned char port_low = 65;

// Connect to the Minecraft server and send a Handshake packet.
int minecraft_connect(Task task) {
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(task.address);

  int err = connect(task.socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (err == -1) {
    // Ignoring connection error logging.
    return -1;
  }

  char buffer[32];
  int packet_len = handshake(task.address, buffer);

  err = send(task.socket_fd, buffer, packet_len, 0);
  if (err == -1) {
    ERROR("CONNECT", "Sending handshake to '%s'", task.address);
    return -1;
  }

  return 0;
}

// Send a status request to the Minecraft server.
char* minecraft_scan(Task task) {
  char request[] = {0x1, 0x0};

  int err = send(task.socket_fd, request, 2, 0);
  if (err == -1) {
    ERROR("SCAN", "Sending status request to '%s'", task.address);
    return NULL;
  }

  int packet_len = read_varint(task.socket_fd);
  if (packet_len == -1) {
    ERROR("SCAN", "Failed to read packet length from '%s'", task.address);
    return NULL;
  }

  if (packet_len > MAX_LENGTH) {
    ERROR("SCAN", "Packet length too large from '%s'", task.address);
    return NULL;
  }

  char packet_id = -1;
  if (read_byte(task.socket_fd, &packet_id) == 0) {
    ERROR("SCAN", "Failed to read packet ID from '%s'", task.address);
    return NULL;
  }

  if (packet_id != 0) {
    return NULL;
  }

  int json_len = read_varint(task.socket_fd);
  if (json_len == -1) {
    ERROR("SCAN", "Failed to read JSON length from '%s'", task.address);
    return NULL;
  }

  if (json_len > MAX_JSON_LENGTH) {
    ERROR("SCAN", "JSON length too large from '%s'", task.address);
    return NULL;
  }

  json_len++;

  char* buffer = malloc(json_len);

  int n = recv(task.socket_fd, buffer, json_len, 0);
  if (n == -1) {
    ERROR("SCAN", "Reading status response from '%s'", task.address);
    return NULL;
  }

  buffer[n] = '\0';

  INFO("GET", "Success '%s'", task.address);
  return buffer;
}

int handshake(const char* host, char* buffer) {
  int host_len = strlen(host);
  // 1 [packet id] + 2 [protocol version] + 1 [server address] + 2 [port] + 1 [state] = 7
  int packet_len = 7 + host_len;

  int i = 0;

  // Packet length
  buffer[i++] = packet_len;
  // Packet ID
  buffer[i++] = 0;
  // Protocol version
  buffer[i++] = -1;
  buffer[i++] = 1;

  // Host length
  buffer[i++] = host_len;
  // Copy host
  memcpy(buffer + i, host, host_len);
  i += host_len;

  // Port (high byte)
  buffer[i++] = port_high;
  // Port (low byte)
  buffer[i++] = port_low;

  // Next state
  buffer[i] = 1;

  // Total packet length
  return packet_len + 1;
}

int read_byte(const int socket_fd, char* byte) {
  int n = recv(socket_fd, byte, 1, 0);
  if (n == -1) {
    return 1;
  }
  return n;
}

int read_varint(const int socket_fd) {
  int result = 0;
  int position = 0;
  char byte = 0;

  do {
    if (read_byte(socket_fd, &byte) == 0) {
      return -1;
    }

    result |= (byte & 0x7F) << position;

    position += 7;
    if (position >= 32) {
      return -1;
    }
  } while ((byte & 0x80) != 0);

  return result;
}
