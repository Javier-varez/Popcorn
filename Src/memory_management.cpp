#include <stdlib.h>

#include "memory_management.h"
#include "platform.h"
#include "spinlock.h"

static OS::SpinLock lock;

CLINKAGE void* OsMalloc(size_t size)
{
    void *ptr = nullptr;
    // TODO: Migrate to custom allocation scheme
    lock.Lock();
    ptr = malloc(size);
    lock.Unlock();

    return ptr;
}

CLINKAGE void OsFree(void* ptr)
{
    lock.Lock();
    free(ptr);
    lock.Unlock();
}