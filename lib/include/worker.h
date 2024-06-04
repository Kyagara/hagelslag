#ifndef WORKER_H
#define WORKER_H

#include <sqlite3.h>

#include "queue.h"

#ifndef THREADS
#define THREADS 4
#endif

typedef struct {
  Queue* queue;
  // Function responsible for connecting, 'getting' and inserting to the database.
  void (*scan)(sqlite3* db, sqlite3_stmt* insert_stmt, int socket_fd, const char* address);
} WorkerArgs;

void* thread_worker(void* arg);

#endif // WORKER_H
