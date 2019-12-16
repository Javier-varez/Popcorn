
#ifndef MUTEX_H_
#define MUTEX_H_

#include <stdint.h>

namespace OS
{
    class Mutex
    {
    public:
        Mutex();
        void Lock();
        void Unlock();
    private:
        uint8_t available;
    };
}

#endif