#include "syscall.h"
#include "syscall_idx.h"
#include "cortex-m_port.h"
#include "kernel.h"

// Instantiate MCU
static Hw::MCU mcu;
static OS::Kernel kernel;

namespace OS {
    Syscall& Syscall::Instance()
    {
        static Syscall syscall;
        return syscall;
    }

    void Syscall::StartOS()
    {

        Hw::MCU::SupervisorCall<SyscallIdx::StartOS>();
        #ifndef UNITTEST
        while (1); // Should never get here
        #endif
    }

    void Syscall::CreateTask(task_func func, std::uintptr_t arg, enum Priority priority, const char* name, std::uint32_t stack_size)
    {
        Hw::MCU::SupervisorCall<SyscallIdx::CreateTask>();
    }

    void Syscall::DestroyTask()
    {
        Hw::MCU::SupervisorCall<SyscallIdx::DestroyTask>();
    }

    void Syscall::Sleep(std::uint32_t ticks)
    {
        Hw::MCU::SupervisorCall<SyscallIdx::Sleep>();
    }

    void Syscall::Yield()
    {
        Hw::MCU::SupervisorCall<SyscallIdx::Yield>();
    }

    void Syscall::Wait(const Blockable& blockable)
    {
        Hw::MCU::SupervisorCall<SyscallIdx::Wait>();
    }

    void Syscall::Lock(const Blockable& blockable, bool acquired)
    {
        Hw::MCU::SupervisorCall<SyscallIdx::Lock>();
    }

    void Syscall::RegisterError()
    {
        Hw::MCU::SupervisorCall<SyscallIdx::RegisterError>();
    }
}
