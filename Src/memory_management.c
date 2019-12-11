#include "memory_management.h"

static uint8_t memory_pool[MEMORY_POOL_SIZE];
static uint8_t* pool_ptr = memory_pool;

void* OsMalloc(size_t size)
{
    // FIXME: Make sure that concurrent access does not corrupt pool_ptr
    if ((pool_ptr + size) < (&memory_pool[MEMORY_POOL_SIZE]))
    {
        void* ptr = pool_ptr; 
        pool_ptr += size;
        return ptr;
    }
    return NULL;
}

void OsFree(void* ptr)
{
    (void) ptr;
}