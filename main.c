#include <stdio.h>
#include <unistd.h>

#include "connection.h"
#include "logger.h"

int main() {
  log_level_from_env();

  int socketfd = create_socket();
  if (socketfd == -1) {
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

          get(socketfd, ip);
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
