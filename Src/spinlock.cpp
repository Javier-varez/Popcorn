
#include "spinlock.h"
#include "syscall.h"
extern "C" {
#include "cmsis_gcc.h"
}

namespace OS
{
    SpinLock::SpinLock() :
        available(true)
    { }

    void SpinLock::Lock()
    {
        bool done = false;
        while (!done)
        {
            if (__LDREXB(&available))
                done = __STREXB(false, &available) == 0;
        }
        __CLREX();
    }

    void SpinLock::Unlock()
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
                OS::Syscall::Instance().RegisterError();
            }
        }
        __CLREX();
    }
}