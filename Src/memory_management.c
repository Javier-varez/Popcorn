#include "memory_management.h"
#include <stdlib.h>

void* OsMalloc(size_t size)
{
    // TODO: Migrate to custom allocation scheme
    return malloc(size);
}

void OsFree(void* ptr)
{
    free(ptr);
}