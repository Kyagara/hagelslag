#include "queue.h"

#define NUM_THREADS 16

typedef struct {
  Queue* queue;
  pthread_t threads[NUM_THREADS];
} ThreadPool;

ThreadPool new_pool();
void join_threads(ThreadPool pool);
