#include <signal.h>
#include <stdlib.h>

#include "ip.h"
#include "logger.h"
#include "worker.h"

#if SCANNER == HTTP
#include "scanners/http.h"
#elif SCANNER == MINECRAFT
#include "scanners/minecraft.h"
#endif

void new_threadpool(pthread_t* threads, WorkerArgs* args);
void join_threads(pthread_t* threads);
void signal_handler(int sig);

int* run;

int main() {
  log_level_from_env();

  // Handle signals.
  run = malloc(sizeof(int));
  *run = 1;
  signal(SIGINT, signal_handler);

  WorkerArgs* args = malloc(sizeof(WorkerArgs));
  args->queue = new_queue();
  args->scanner = set_scanner();

  init_db_pool();

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

  free_db_pool();
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
