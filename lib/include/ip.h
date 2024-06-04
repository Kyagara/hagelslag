#ifndef IP_H
#define IP_H

#include <stdint.h>

#include "queue.h"

void generate_ips(Queue* queue, int* run);
uint32_t address_to_int(const char* address);

#endif // IP_H
