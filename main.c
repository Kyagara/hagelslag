#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "ip.h"
#include "logger.h"
#include "scanners/http.h"
#include "worker.h"

void new_threadpool(pthread_t* threads, WorkerArgs* args);
void join_threads(pthread_t* threads);

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

  WorkerArgs* args = malloc(sizeof(WorkerArgs));
  args->queue = new_queue();
  args->scan = http_scan;

  // Create a pool of threads.
  pthread_t threads[NUM_THREADS];
  new_threadpool(threads, args);

  signal(SIGINT, signal_handler);

  // Start generating IPs and sending them to the queue.
  generate_ips(args->queue, run);

  INFO("MAIN", "All possible IPs generated, setting done flag in queue");

  // Signal that no more tasks will be added to the queue.
  signal_done(args->queue);

  // Wait until all tasks are completed and threads are done.
  join_threads(threads);

  INFO("MAIN", "All tasks completed, cleaning up");

  free_queue(args->queue);
  free(args);
  free(run);
  return 0;
}

void new_threadpool(pthread_t* threads, WorkerArgs* args) {
  INFO("THREAD", "Creating %d threads", NUM_THREADS);

  for (int i = 0; i < NUM_THREADS; i++) {
    int err = pthread_create(&threads[i], NULL, thread_worker, args);
    if (err) {
      FATAL("THREAD", "Creating thread ID '%d'", i);
    }

    threads[i] = threads[i];
  }
}

void join_threads(pthread_t* threads) {
  for (int i = 0; i < NUM_THREADS; i++) {
    INFO("MAIN", "Waiting for thread '%d'", i);
    pthread_join(threads[i], NULL);
    INFO("MAIN", "Joined thread '%d'", i);
  }
}
