#include "memory_management.h"
#include "platform.h"
#include <stdlib.h>



CLINKAGE void* OsMalloc(size_t size)
{
    // TODO: Migrate to custom allocation scheme
    return malloc(size);
}

CLINKAGE void OsFree(void* ptr)
{
    free(ptr);
}