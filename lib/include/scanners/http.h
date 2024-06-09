#ifndef HTTP_H
#define HTTP_H

#include "queue.h"

int http_connect(Task task);
char* http_scan(Task task);

#endif // HTTP_H
