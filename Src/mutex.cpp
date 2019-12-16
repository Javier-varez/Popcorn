
#include "mutex.h"
extern "C" {
#include "cmsis_gcc.h"
}

namespace OS
{
    Mutex::Mutex() :
        available(true)
    { }

    void Mutex::Lock()
    {
        bool done = false;
        while (!done)
        {
            if (__LDREXB(&available))
                done = __STREXB(false, &available) == 0;
        }
        __CLREX();
    }

    void Mutex::Unlock()
    {
        bool done = false;
        while (!done)
        {
            if (!__LDREXB(&available))
                done = __STREXB(true, &available) == 0;
            else
                while(true); // Already released!
        }
        __CLREX();
    }
}