#ifndef WORKER_H
#define WORKER_H

#include <sqlite3.h>

#include "queue.h"
#include "scanner.h"

#ifndef THREADS
#define THREADS 4
#endif

typedef struct {
  // Queue shared between all threads.
  Queue* queue;

  Scanner scanner;
} WorkerArgs;

void* thread_worker(void* arg);

#endif // WORKER_H
