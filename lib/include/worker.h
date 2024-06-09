#ifndef WORKER_H
#define WORKER_H

#include "queue.h"
#include "scanner.h"

#ifndef THREADS
#define THREADS 4
#endif

#ifndef DATABASE_URI
#define DATABASE_URI "mongodb://127.0.0.1:27017/hagelslag"
#endif

typedef struct {
  // Queue shared between all threads.
  Queue* queue;

  Scanner scanner;
} WorkerArgs;

void* thread_worker(void* arg);
void init_db_pool();
void free_db_pool();

#endif // WORKER_H
