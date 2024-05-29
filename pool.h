#include "types.h"

ThreadPool *create_pool();
void free_pool(ThreadPool *pool);
void submit_task(ThreadPool *pool, const char *ip);
