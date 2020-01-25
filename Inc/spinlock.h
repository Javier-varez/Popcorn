
#ifndef MUTEX_H_
#define MUTEX_H_

#include <stdint.h>

namespace OS
{
    class SpinLock
    {
    public:
        SpinLock();
        void Lock();
        void Unlock();
    private:
        uint8_t available;
    };
}

#endif