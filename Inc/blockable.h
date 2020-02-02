#ifndef BLOCKABLE_H_
#define BLOCKABLE_H_

#include "syscall.h"

namespace OS
{
    // Mutex, lists, etc, must be sublcass of Blockable
    class Blockable {
    public:
        virtual bool IsBlocked() const
        {
            return false;
        };
    protected:
        void Block()
        {
            Syscall::Instance().Wait(*this);
        }
    };
} // namespace OS

#endif