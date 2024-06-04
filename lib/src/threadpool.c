#include <unistd.h>

#include "connection.h"
#include "database.h"
#include "logger.h"
#include "threadpool.h"

void* thread_worker(void* arg);

ThreadPool new_pool() {
  ThreadPool pool;
  pthread_t threads[NUM_THREADS];

  Queue* queue = new_queue();
  pool.queue = queue;

  INFO("THREAD", "Creating %d threads", NUM_THREADS);

  for (int i = 0; i < NUM_THREADS; i++) {
    int err = pthread_create(&threads[i], NULL, thread_worker, queue);
    if (err) {
      FATAL("THREAD", "Creating thread ID '%d'", i);
    }

    pool.threads[i] = threads[i];
  }

  return pool;
}

void* thread_worker(void* arg) {
  Queue* queue = (Queue*)arg;
  sqlite3* db;

  int err = sqlite3_open_v2(DATABASE_NAME, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, NULL);
  if (err != SQLITE_OK) {
    FATAL("DATABASE", "Can't open database connection: %s", sqlite3_errmsg(db));
  }

  sqlite3_stmt* conn_stmt = insert_conn_statement(db);
  sqlite3_stmt* get_stmt = insert_get_statement(db);

  Task tasks[MAXIMUM_TASKS_PER_THREAD];
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
    while (queue->size > 0 && current_tasks < MAXIMUM_TASKS_PER_THREAD) {
      Task task = queue->tasks[queue->front];
      queue->front = (queue->front + 1) % MAXIMUM_TASKS;
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
    while (current_tasks > 0) {
      scan(db, conn_stmt, get_stmt, tasks[n].socket_fd, tasks[n].address);
      close(tasks[n].socket_fd);
      current_tasks--;
      n++;
    }
  }

  sqlite3_finalize(conn_stmt);
  sqlite3_finalize(get_stmt);
  sqlite3_close(db);
  return NULL;
}

void join_threads(ThreadPool pool) {
  for (int i = 0; i < NUM_THREADS; i++) {
    INFO("MAIN", "Waiting for thread '%d'", i);
    pthread_join(pool.threads[i], NULL);
    INFO("MAIN", "Joined thread '%d'", i);
  }
}
