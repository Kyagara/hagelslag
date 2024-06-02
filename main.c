#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "logger.h"
#include "pool.h"

int is_reserved(int* seg_a, int* seg_b, int* seg_c);
void get_starting_ip(int* seg_a, int* seg_b, int* seg_c, int* seg_d);

int run = 1;

void signal_handler(int sig) {
  INFO("MAIN", "Signal '%d' received, exiting", sig);
  run = 0;
}

int main() {
  int log_level = log_level_from_env();
  INFO("MAIN", "Log level set to '%d'", log_level);

  // Starting a database connection to create tables.
  create_tables();

  // Create a pool of threads and the Queue.
  ThreadPool pool = new_pool();

  int seg_a = 1, seg_b = 0, seg_c = 0, seg_d = 0;
  get_starting_ip(&seg_a, &seg_b, &seg_c, &seg_d);

  INFO("MAIN", "Starting IP set to '%d.%d.%d.%d'", seg_a, seg_b, seg_c, seg_d);

  signal(SIGINT, signal_handler);

  // Generating all possible IP addresses.
  while (run) {
    char ip[16];
    snprintf(ip, sizeof(ip), "%d.%d.%d.%d", seg_a, seg_b, seg_c, seg_d);

    // Check if the current IP is reserved.
    if (is_reserved(&seg_a, &seg_b, &seg_c) == 1) {
      INFO("MAIN", "Reserved range reached, skipping to next IP");
      continue;
    }

    DEBUG("MAIN", "Submitting task '%s'", ip);

    submit_task(pool.queue, ip);

    // 0.0.0.x
    seg_d++;
    if (seg_d > 255) {
      seg_d = 0;
      // 0.0.x.0
      seg_c++;

      if (seg_c > 255) {
        seg_c = 0;
        // 0.x.0.0
        seg_b++;

        if (seg_b > 255) {
          seg_b = 0;
          // x.0.0.0
          seg_a++;

          // 224.x.x.x and above are reserved.
          if (seg_a >= 224) {
            // All possible IPs generated, exit loop.
            break;
          }
        }
      }
    }
  }

  INFO("MAIN", "All possible IPs generated, setting done flag in queue");

  // Signal that no more tasks will be added to the queue.
  signal_done(pool.queue);

  // Wait until all tasks are completed and threads are done.
  join_threads(pool);

  free_queue(pool.queue);
  return 0;
}

// Check if the IP is in any reserved range, return 1 if it is, skips to the next available range.
int is_reserved(int* seg_a, int* seg_b, int* seg_c) {
  if (*seg_a == 10 || *seg_a == 127) {
    *seg_a = *seg_a + 1;
    return 1;
  }

  // 169.254.x.x
  if (*seg_a == 169 && *seg_b == 254) {
    *seg_b = 255;
    return 1;
  }

  // 172.(>= 16 && <= 31).x.x
  if (*seg_a == 172 && *seg_b >= 16 && *seg_b <= 31) {
    *seg_b = 32;
    return 1;
  }

  if (*seg_a == 192) {
    // 192.0.0.x
    // 192.0.2.x
    if (*seg_b == 0) {
      if (*seg_c == 0 || *seg_c == 2) {
        *seg_c = *seg_c + 1;
        return 1;
      }

      return 0;
    }

    // 192.88.99.0
    if (*seg_b == 88 && *seg_c == 99) {
      *seg_c = 100;
      return 1;
    }

    // 192.168.x.x
    if (*seg_b == 168) {
      *seg_b = 169;
      return 1;
    }

    return 0;
  }

  // 198.51.100.x
  if (*seg_a == 198 && *seg_b == 51 && *seg_c == 100) {
    *seg_c = 101;
    return 1;
  }

  // 203.0.113.x
  if (*seg_a == 203 && *seg_b == 0 && *seg_c == 113) {
    *seg_c = 114;
    return 1;
  }

  return 0;
}

void get_starting_ip(int* seg_a, int* seg_b, int* seg_c, int* seg_d) {
  char* starting_ip = getenv("IP");
  if (starting_ip != NULL) {
    *seg_a = atoi(strtok(starting_ip, "."));
    *seg_b = atoi(strtok(NULL, "."));
    *seg_c = atoi(strtok(NULL, "."));
    *seg_d = atoi(strtok(NULL, "."));
  }
}
