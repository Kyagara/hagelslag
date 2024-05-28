#include <stdio.h>
#include <unistd.h>

#include "connection.h"
#include "logger.h"

FILE *exists(const char *filepath);

int main() {
  log_level_from_env();

  int socketfd = create_socket();
  if (socketfd == -1) {
    return -1;
  }

  FILE *connect_results = exists("connect_results.txt");
  if (!connect_results) {
    return -1;
  }

  FILE *get_results = exists("get_results.txt");
  if (!get_results) {
    return -1;
  }

  char ip[16];
  int seg_a, seg_b, seg_c, seg_d = 0;

  for (seg_a = 0; seg_a <= 255; seg_a++) {
    for (seg_b = 0; seg_b <= 255; seg_b++) {
      for (seg_c = 0; seg_c <= 255; seg_c++) {
        for (seg_d = 0; seg_d <= 255; seg_d++) {
          snprintf(ip, sizeof(ip), "%d.%d.%d.%d", seg_a, seg_b, seg_c, seg_d);

          debug("TRY", ip);

          int err = conn(socketfd, ip);
          if (err == -1) {
            continue;
          }

          fprintf(connect_results, "%s\n", ip);

          err = get(socketfd, ip);
          if (err == -1) {
            continue;
          }

          fprintf(get_results, "%s\n", ip);
        }

        info("SEGMENT", "DONE %d.%d.%d.X", seg_a, seg_b, seg_c);
      }

      info("SEGMENT", "DONE %d.%d.X.X", seg_a, seg_b);
    }

    info("SEGMENT", "DONE %d.X.X.X", seg_a);
  }

  info("SEGMENT", "DONE X.X.X.X");

  close(socketfd);

  info("SOCKET", "CLOSE");
  return 0;
}

// Check if a file exists, if not, create it, returns the file pointer, or NULL
// on error.
FILE *exists(const char *filepath) {
  FILE *fp = fopen(filepath, "r");
  if (!fp) {
    debug("FILE", "NOT FOUND %s", filepath);

    FILE *fp = fopen(filepath, "w");
    if (!fp) {
      error("FILE", "CREATE %s", filepath);
      return NULL;
    }

    info("FILE", "CREATED %s", filepath);
    return fp;
  }

  debug("FILE", "FOUND %s", filepath);

  fp = fopen(filepath, "w");
  if (!fp) {
    error("FILE", "RECREATE %s", filepath);
    return NULL;
  }

  info("FILE", "RECREATED %s", filepath);
  return fp;
}
