#ifndef MUTEX_H_
#define MUTEX_H_

#include <cstdint>
#include "blockable.h"

namespace OS
{
    class Mutex: Blockable
    {
    public:
        Mutex();
        void Lock();
        void Unlock();
    private:
        std::uint8_t available;
        bool IsBlocked() const override;
    };
}

#endif