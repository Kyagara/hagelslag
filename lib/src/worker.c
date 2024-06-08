#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "database.h"
#include "logger.h"
#include "scanner.h"
#include "worker.h"

int create_socket();

void* thread_worker(void* arg) {
  WorkerArgs* args = (WorkerArgs*)arg;
  Queue* queue = args->queue;
  Scanner scanner = args->scanner;

  sqlite3* db;

  int err = sqlite3_open_v2(DATABASE_URI, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, NULL);
  if (err != SQLITE_OK) {
    FATAL("DATABASE", "Can't open database connection: %s", sqlite3_errmsg(db));
  }

  sqlite3_stmt* insert_stmt = insert_statement(db);

  Task tasks[TASKS_PER_THREAD];
  Result results[TASKS_PER_THREAD];

  // Current number of tasks that are being worked on by this thread.
  int current_tasks = 0;

  while (1) {
    pthread_mutex_lock(&queue->mutex);

    // Wait until there is a task in the queue.
    while (queue->size == 0 && !queue->done) {
      pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }

    // If the queue is empty and all tasks have already been sent, break.
    if (queue->size == 0 && queue->done) {
      pthread_mutex_unlock(&queue->mutex);
      break;
    }

    // Retrieve all available tasks from the queue.
    while (queue->size > 0 && current_tasks < TASKS_PER_THREAD) {
      Task task = queue->tasks[queue->front];
      queue->front = (queue->front + 1) % QUEUE_LIMIT;
      queue->size--;

      // Signal that a slot is available in the queue.
      pthread_cond_signal(&queue->not_full);

      task.socket_fd = create_socket();
      if (task.socket_fd == -1) {
        continue;
      }

      tasks[current_tasks] = task;

      current_tasks++;
    }

    pthread_mutex_unlock(&queue->mutex);

    // Work on the tasks, blocking until all tasks in this thread are done.

    int n = 0;
    int successful_results = 0;
    while (current_tasks > 0) {
      // Connect.
      err = scanner.connect(tasks[n].socket_fd, tasks[n].address);
      if (err == 0) {
        // Scan.
        err = scanner.scan(tasks[n].socket_fd, tasks[n].address, &results[successful_results]);
        if (err == 0) {
          // Add to successful results so it can be saved in the database.
          successful_results++;
        }
      }

      close(tasks[n].socket_fd);
      current_tasks--;
      n++;
    }

    int result = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    if (result != SQLITE_OK) {
      FATAL("DATABASE", "Can't start transaction: %s", sqlite3_errmsg(db));
    }

    while (successful_results > 0) {
      scanner.save(db, insert_stmt, results[successful_results - 1].address);
      INFO("DATABASE", "Saved '%s'", results[successful_results - 1].address);
      successful_results--;
    }

    result = sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    if (result != SQLITE_OK) {
      FATAL("DATABASE", "Can't commit transaction: %s", sqlite3_errmsg(db));
    }
  }

  sqlite3_finalize(insert_stmt);
  sqlite3_close(db);
  return NULL;
}

// Creates a socket, sets a timeout of 1 second, returns -1 on error or socket_fd.
int create_socket() {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    ERROR("SOCKET", "Creating socket");
    return -1;
  }

  struct timeval timeval;
  timeval.tv_sec = 1;
  timeval.tv_usec = 0;

  setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeval, sizeof(timeval));
  setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeval, sizeof(timeval));

  return socket_fd;
}
