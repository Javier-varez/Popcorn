
#include "spinlock.h"

namespace OS
{
    SpinLock::SpinLock() :
        available(true)
    { }

    void SpinLock::Lock()
    {
        while (!available);
        available = false;
    }

    void SpinLock::Unlock()
    {
        available = true;
    }
}