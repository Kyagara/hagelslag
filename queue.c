#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

Queue *new_queue() {
  Queue *queue = malloc(sizeof(Queue));
  queue->size = 0;
  queue->front = 0;
  queue->rear = 0;
  queue->done = 0;
  pthread_mutex_init(&queue->mutex, NULL);
  pthread_cond_init(&queue->not_empty, NULL);
  pthread_cond_init(&queue->not_full, NULL);
  return queue;
}

// Submit a task to the queue.
void submit_task(Queue *queue, const char *ip) {
  pthread_mutex_lock(&queue->mutex);

  // Wait until there is a slot available in the queue.
  while (queue->size >= MAXIMUM_TASKS) {
    pthread_cond_wait(&queue->not_full, &queue->mutex);
  }

  // Add this task to the queue.
  strcpy(queue->tasks[queue->rear].address, ip);
  queue->rear = (queue->rear + 1) % MAXIMUM_TASKS;
  queue->size++;

  // Signal that a task is available in the queue.
  pthread_cond_signal(&queue->not_empty);
  pthread_mutex_unlock(&queue->mutex);
}

void signal_done(Queue *queue) {
  pthread_mutex_lock(&queue->mutex);
  queue->done = 1;
  pthread_cond_broadcast(&queue->not_empty);
  pthread_mutex_unlock(&queue->mutex);
}

void free_queue(Queue *queue) {
  pthread_mutex_destroy(&queue->mutex);
  pthread_cond_destroy(&queue->not_empty);
  pthread_cond_destroy(&queue->not_full);
  free(queue);
}
