#include <pthread.h>

#define NUM_THREADS 16
#define QUEUE_SIZE 16

// Structure to hold task information.
typedef struct {
  char ip[16];
} Task;

// Queue structure for holding tasks.
typedef struct {
  // Current queue size.
  int size;
  // Indexes.
  int front, rear;
  Task tasks[QUEUE_SIZE];
  pthread_cond_t empty;
  pthread_cond_t full;
  pthread_mutex_t mutex;
} ThreadPool;
