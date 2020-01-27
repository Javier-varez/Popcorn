#ifndef MUTEX_H_
#define MUTEX_H_

#include <cstdint>
#include "os.h"

namespace OS
{
    class Mutex: Blockable
    {
    public:
        Mutex();
        void lock();
        void unlock();
    private:
        std::uint8_t available;
        bool IsBlocked() const override;
    };
}

#endif