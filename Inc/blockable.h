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
        void LockAcquired()
        {
            Syscall::Instance().Lock(*this, true);
        }
        void LockReleased()
        {
            Syscall::Instance().Lock(*this, false);
        }
        struct task_control_block *blocker;
        friend class Kernel;
    };
} // namespace OS

#endif