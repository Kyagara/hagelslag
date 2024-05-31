#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "connection.h"
#include "logger.h"
#include "pool.h"

void *thread_worker(void *arg);

ThreadPool new_pool() {
  ThreadPool pool;
  pthread_t threads[NUM_THREADS];

  Queue *queue = new_queue();
  pool.queue = queue;

  info("THREAD", "Creating %d threads", NUM_THREADS);

  for (int i = 0; i < NUM_THREADS; i++) {
    int err = pthread_create(&threads[i], NULL, thread_worker, queue);
    if (err) {
      fatal("THREAD", "Creating thread ID '%d'", i);
    }

    pool.threads[i] = threads[i];
  }

  return pool;
}

void *thread_worker(void *arg) {
  Queue *queue = (Queue *)arg;

  Task tasks[MAXIMUM_TASKS_PER_THREAD];
  // Current number of tasks that are being worked on by this thread.
  int current_tasks = 0;

  while (1) {
    pthread_mutex_lock(&queue->mutex);

    // Wait until there is a task in the queue.
    while (queue->size == 0 && queue->done == 0) {
      pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }

    // If the queue is empty and all tasks have already been sent, break.
    if (queue->size == 0 && queue->done == 1) {
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
      // try_connection(tasks[i].socket_fd, tasks[i].address);
      close(tasks[n].socket_fd);
      current_tasks--;
      n++;
    }
  }

  return NULL;
}

void join_threads(ThreadPool pool) {
  for (int i = 0; i < NUM_THREADS; i++) {
    info("MAIN", "Waiting for thread '%d'", i);
    pthread_join(pool.threads[i], NULL);
    info("MAIN", "Joined thread '%d'", i);
  }
}
