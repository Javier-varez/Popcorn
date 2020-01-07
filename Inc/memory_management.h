
#ifndef MEMORY_MANAGEMENT_H_
#define MEMORY_MANAGEMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define MEMORY_POOL_SIZE        (4096)

void* OsMalloc(size_t size);
void OsFree(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // MEMORY_MANAGEMENT_H_