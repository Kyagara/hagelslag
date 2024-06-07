#ifndef WORKER_H
#define WORKER_H

#include <sqlite3.h>

#include "queue.h"

#ifndef THREADS
#define THREADS 4
#endif

typedef void Scan(sqlite3* db, sqlite3_stmt* insert_stmt, int socket_fd, const char* address);

typedef struct {
  // Queue shared between all threads.
  Queue* queue;
  // Function responsible for connecting, 'getting' and inserting to the database.
  Scan* scan;
} WorkerArgs;

void* thread_worker(void* arg);

#endif // WORKER_H
