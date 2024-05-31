#include <pthread.h>
#include <stdio.h>

#include "logger.h"

FILE *connect_results;
FILE *get_results;

pthread_mutex_t connect_results_mutex;
pthread_mutex_t get_results_mutex;

// Check if a file exists, if not, create it.
// Returns the file pointer, or NULL on error.
FILE *exists(const char *filepath) {
  FILE *fp = fopen(filepath, "w");
  if (!fp) {
    ERROR("FILE", "Creating '%s'", filepath);
    return NULL;
  }

  INFO("FILE", "Recreated '%s'", filepath);
  return fp;
}

void load_files() {
  connect_results = exists("connect_results.txt");
  get_results = exists("get_results.txt");
  pthread_mutex_init(&connect_results_mutex, NULL);
  pthread_mutex_init(&get_results_mutex, NULL);
}

void free_files() {
  pthread_mutex_destroy(&connect_results_mutex);
  pthread_mutex_destroy(&get_results_mutex);
  fclose(connect_results);
  fclose(get_results);
}

void save_connection(char *ip) {
  pthread_mutex_lock(&connect_results_mutex);
  fprintf(connect_results, "%s\n", ip);
  fflush(connect_results);
  pthread_mutex_unlock(&connect_results_mutex);
}

void save_get(char *ip) {
  pthread_mutex_lock(&get_results_mutex);
  fprintf(get_results, "%s\n", ip);
  fflush(get_results);
  pthread_mutex_unlock(&get_results_mutex);
}