
#ifndef SYSCALL_IDX_H_
#define SYSCALL_IDX_H_

namespace OS
{
    enum class SyscallIdx
    {
        StartOS,
        CreateTask,
        Sleep,
        DestroyTask,
        Yield,
        Wait,
        RegisterError,
        Lock
    };
}

#endif