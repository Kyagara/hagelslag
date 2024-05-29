#include <stdio.h>

#include "connection.h"
#include "file.h"
#include "logger.h"
#include "pool.h"

int main() {
  log_level_from_env();
  load_files();

  ThreadPool *pool = create_pool();
  if (pool == NULL) {
    return -1;
  }

  int seg_a, seg_b, seg_c, seg_d = 0;

  // Generating all possible IP addresses.
  // This looks funny.
  for (seg_a = 0; seg_a <= 255; seg_a++) {
    for (seg_b = 0; seg_b <= 255; seg_b++) {
      for (seg_c = 0; seg_c <= 255; seg_c++) {
        for (seg_d = 0; seg_d <= 255; seg_d++) {
          char ip[16];
          snprintf(ip, sizeof(ip), "%d.%d.%d.%d", seg_a, seg_b, seg_c, seg_d);

          debug("TRY", ip);

          submit_task(pool, ip);
        }
        info("SEGMENT", "DONE %d.%d.%d.X", seg_a, seg_b, seg_c);
      }
      info("SEGMENT", "DONE %d.%d.X.X", seg_a, seg_b);
    }
    info("SEGMENT", "DONE %d.X.X.X", seg_a);
  }
  info("SEGMENT", "DONE X.X.X.X");

  free_files();
  free_pool(pool);
  return 0;
}
