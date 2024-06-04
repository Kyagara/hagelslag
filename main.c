#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "ip.h"
#include "logger.h"

int* run;

void signal_handler(int sig) {
  INFO("MAIN", "Signal '%d' received, exiting", sig);
  *run = 0;
}

int main() {
  run = malloc(sizeof(int));
  *run = 1;

  int log_level = log_level_from_env();
  INFO("MAIN", "Log level set to '%d'", log_level);

  // Starting a database connection to create tables.
  create_tables();

  // Create a pool of threads and the Queue.
  ThreadPool pool = new_pool();

  signal(SIGINT, signal_handler);

  // Start generating IPs and sending them to the queue.
  generate_ips(pool, run);

  INFO("MAIN", "All possible IPs generated, setting done flag in queue");

  // Signal that no more tasks will be added to the queue.
  signal_done(pool.queue);

  // Wait until all tasks are completed and threads are done.
  join_threads(pool);

  free_queue(pool.queue);
  free(run);
  return 0;
}
