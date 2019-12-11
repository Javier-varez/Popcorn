
#ifndef MEMORY_MANAGEMENT_H_
#define MEMORY_MANAGEMENT_H_

#include <stddef.h>
#include <stdint.h>

#define MEMORY_POOL_SIZE        (4096)

void* OsMalloc(size_t size);
void OsFree(void* ptr);

#endif // MEMORY_MANAGEMENT_H_