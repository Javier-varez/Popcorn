#include "mutex.h"
extern "C" {
#include "cmsis_gcc.h"
}

namespace OS
{
    Mutex::Mutex() : available(true)
    {

    }

    void Mutex::lock()
    {
        bool done = false;
        do
        {
            if (__LDREXB(&available))
            {
                done = __STREXB(false, &available) == 0;
            }
            else
            {
                // Sleep until the lock is free
                Block();
            }
        } while (!done);
        __CLREX();
    }

    void Mutex::unlock()
    {
        bool done = false;
        while (!done)
        {
            if (!__LDREXB(&available))
            {
                done = __STREXB(true, &available) == 0;
            }
            else
            {
                OS::Scheduler::RegisterError();
            }
        }
        __CLREX();
        OS::Scheduler::Yield();
    }

    bool Mutex::IsBlocked() const {
        return !available;
        // This should only be called within the os.
        // No exclusive access required
    }

}