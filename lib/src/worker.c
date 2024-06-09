#include <mongoc/mongoc.h>

#include "logger.h"
#include "scanner.h"
#include "worker.h"

int create_socket();

mongoc_client_pool_t* pool;
mongoc_uri_t* uri;

void* thread_worker(void* arg) {
  WorkerArgs* args = (WorkerArgs*)arg;
  Queue* queue = args->queue;
  Scanner scanner = args->scanner;

#if SCANNER == HTTP
  const char c[4] = "http";
#elif SCANNER == MINECRAFT
  const char c[9] = "minecraft";
#endif

  mongoc_client_t* client = mongoc_client_pool_pop(pool);
  mongoc_collection_t* collection = mongoc_client_get_collection(client, "hagelslag", c);

  Task tasks[TASKS_PER_THREAD];

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
    while (current_tasks > 0) {
      int err = scanner.connect(tasks[n]);
      if (err == 0) {
        char* data = scanner.scan(tasks[n]);

        if (data != NULL) {
          bson_t* doc = bson_new();
          BSON_APPEND_UTF8(doc, "_id", tasks[n].address);
          BSON_APPEND_UTF8(doc, "data", data);

          bson_error_t error;
          if (!mongoc_collection_insert_one(collection, doc, NULL, NULL, &error)) {
            ERROR("DATABASE", "Can't insert document: %s", error.message);
          }

          bson_destroy(doc);
          free(data);
        }
      }

      close(tasks[n].socket_fd);
      current_tasks--;
      n++;
    }
  }

  mongoc_client_pool_push(pool, client);
  mongoc_collection_destroy(collection);
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

void init_db_pool() {
  mongoc_init();
  uri = mongoc_uri_new(DATABASE_URI);
  pool = mongoc_client_pool_new(uri);
}

void free_db_pool() {
  mongoc_client_pool_destroy(pool);
  mongoc_uri_destroy(uri);
  mongoc_cleanup();
}
