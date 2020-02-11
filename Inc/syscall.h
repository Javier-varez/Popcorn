
#ifndef SYSCALL_H_
#define SYSCALL_H_

#include <cstdint>

typedef void (*task_func)(void*);

// Application Hooks
void App_SysTick_Hook();

class SyscallTest;
namespace OS {
    enum class Priority
    {
        IDLE = 0,
        Level_0,
        Level_1,
        Level_2,
        Level_3,
        Level_4,
        Level_5,
        Level_6,
        Level_7,
        Level_8,
        Level_9
    };

    class Blockable;
    class Syscall {
    public:
        void CreateTask(task_func func, std::uintptr_t arg, enum Priority priority, const char* name, std::uint32_t stack_size);
        void DestroyTask();
        void Sleep(std::uint32_t ticks);
        void StartOS();
        void Yield();
        void RegisterError();
        void Lock(const Blockable& blockable, bool acquired);
        static Syscall& Instance();

    private:
        void Wait(const Blockable&);
        friend class Blockable;
        friend class ::SyscallTest;
    };
}

#endif // OS_H_