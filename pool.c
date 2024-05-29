#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "logger.h"
#include "types.h"

void *thread_worker(void *arg);

ThreadPool *create_pool() {
  ThreadPool *pool = malloc(sizeof(ThreadPool));
  if (pool == NULL) {
    error("THREADPOOL", "CREATE");
    return NULL;
  }

  pool->size = 0;
  pool->front = 0;
  pool->rear = 0;

  pthread_mutex_init(&pool->mutex, NULL);
  pthread_cond_init(&pool->empty, NULL);
  pthread_cond_init(&pool->full, NULL);

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_t thread;
    int err = pthread_create(&thread, NULL, thread_worker, pool);
    if (err != 0) {
      fatal("THREAD", "CREATE");
    } else {
      err = pthread_detach(thread);
      if (err != 0) {
        fatal("THREAD", "DETACH");
      }
    }
  }

  return pool;
}

void free_pool(ThreadPool *pool) {
  pthread_mutex_lock(&pool->mutex);
  pthread_cond_broadcast(&pool->empty);
  pthread_mutex_unlock(&pool->mutex);
  pthread_mutex_destroy(&pool->mutex);
  pthread_cond_destroy(&pool->empty);
  pthread_cond_destroy(&pool->full);
  free(pool);
}

// Worker function for threads.
void *thread_worker(void *arg) {
  ThreadPool *pool = (ThreadPool *)arg;

  while (1) {
    pthread_mutex_lock(&pool->mutex);

    // Wait until there are tasks in the pool.
    while (pool->size == 0) {
      pthread_cond_wait(&pool->empty, &pool->mutex);
    }

    // Retrieve and remove a task from the pool.
    Task task = pool->tasks[pool->front];
    pool->front = (pool->front + 1) % QUEUE_SIZE;
    pool->size--;

    pthread_cond_signal(&pool->full);
    pthread_mutex_unlock(&pool->mutex);

    int socketfd = create_socket();
    if (socketfd == -1) {
      error("SOCKET", "CREATE");
      continue;
    }

    try_connection(socketfd, task.ip);

    close(socketfd);
  }

  return NULL;
}

// Submit a task to the thread pool.
void submit_task(ThreadPool *pool, const char *ip) {
  pthread_mutex_lock(&pool->mutex);

  // Wait until there is space in the pool.
  while (pool->size >= QUEUE_SIZE) {
    pthread_cond_wait(&pool->full, &pool->mutex);
  }

  // Add the task to the pool.
  strcpy(pool->tasks[pool->rear].ip, ip);
  pool->rear = (pool->rear + 1) % QUEUE_SIZE;
  pool->size++;

  pthread_cond_signal(&pool->empty);
  pthread_mutex_unlock(&pool->mutex);
}
