#include "MockMemManagement.h"

StrictMock<MockMemManagement> *g_MockMemManagement = nullptr;

extern "C" void* OsMalloc(std::size_t size)
{
    if (g_MockMemManagement != nullptr) {
        return g_MockMemManagement->Malloc(size);
    }
    return nullptr;
}

extern "C" void OsFree(void *ptr)
{
    if (g_MockMemManagement != nullptr) {
        g_MockMemManagement->Free(ptr);
    }
}