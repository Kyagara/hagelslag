#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "connection.h"
#include "file.h"
#include "logger.h"
#include "pool.h"

int main() {
  int log_level = log_level_from_env();
  INFO("MAIN", "Log level set to '%d'", log_level);

  load_files();

  // Create a pool of threads and the Queue.
  ThreadPool pool = new_pool();

  int seg_a = 0, seg_b = 0, seg_c = 0, seg_d = 0;

  char *starting_ip = getenv("IP");
  if (starting_ip != NULL) {
    seg_a = atoi(strtok(starting_ip, "."));
    seg_b = atoi(strtok(NULL, "."));
    seg_c = atoi(strtok(NULL, "."));
    seg_d = atoi(strtok(NULL, "."));
  }

  // Generating all possible IP addresses.
  while (1) {
    char ip[16];
    snprintf(ip, sizeof(ip), "%d.%d.%d.%d", seg_a, seg_b, seg_c, seg_d);
    submit_task(pool.queue, ip);

    seg_d++;
    if (seg_d > 255) {
      seg_d = 0;
      seg_c++;
      break;
      if (seg_c > 255) {
        seg_c = 0;
        seg_b++;
        if (seg_b > 255) {
          seg_b = 0;
          seg_a++;
          if (seg_a > 255) {
            // All possible IPs generated, exit loop.
            break;
          }
        }
      }
    }
  }

  // Signal that no more tasks will be added to the queue.
  signal_done(pool.queue);

  // Wait until all tasks are completed and threads are done.
  join_threads(pool);

  free_queue(pool.queue);
  free_files();
  return 0;
}
