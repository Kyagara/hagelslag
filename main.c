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
void signal_handler(int sig);

int* run;

int main() {
  int log_level = log_level_from_env();
  INFO("MAIN", "Log level set to '%d'", log_level);

  // Handle signals.
  run = malloc(sizeof(int));
  *run = 1;
  signal(SIGINT, signal_handler);

  // Start a short database connection just to create the tables.
  create_tables();

  WorkerArgs* args = malloc(sizeof(WorkerArgs));
  args->queue = new_queue();
  args->scanner = get_scanner();

  INFO("MAIN", "Using '%s' scanner", SCANNER);

  // Create a pool of threads.
  pthread_t threads[THREADS];
  new_threadpool(threads, args);

  // Start generating IPs and sending them to the queue.
  generate_ips(args->queue, run);

  INFO("MAIN", "Setting 'done' flag in queue");

  // Signal that no more tasks will be added to the queue.
  signal_done(args->queue);

  join_threads(threads);

  INFO("MAIN", "All tasks completed, cleaning up");

  free_queue(args->queue);
  free(args);
  free(run);
  return 0;
}

void new_threadpool(pthread_t* threads, WorkerArgs* args) {
  INFO("THREAD", "Creating %d threads", THREADS);

  for (int i = 0; i < THREADS; i++) {
    int err = pthread_create(&threads[i], NULL, thread_worker, args);
    if (err) {
      FATAL("THREAD", "Creating thread ID '%d'", i);
    }

    threads[i] = threads[i];
  }
}

void join_threads(pthread_t* threads) {
  for (int i = 0; i < THREADS; i++) {
    INFO("MAIN", "Waiting for thread '%d'", i);
    pthread_join(threads[i], NULL);
    INFO("MAIN", "Joined thread '%d'", i);
  }
}

void signal_handler(int sig) {
  INFO("MAIN", "Signal '%d' received, exiting", sig);
  *run = 0;
}
