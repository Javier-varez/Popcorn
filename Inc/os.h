
#ifndef OS_H_
#define OS_H_

#include <cstdint>

constexpr std::uint32_t TASK_STACK_SIZE = 256;
constexpr std::uint32_t MAX_TASK_NAME = 10;

typedef void (*task_func)(void*);

namespace OS {
    class Scheduler {
    public:
        enum PriorityLevel
        {
            TASK_PRIO_IDLE = 0,
            TASK_PRIO_0,
            TASK_PRIO_1,
            TASK_PRIO_2,
            TASK_PRIO_3,
            TASK_PRIO_4,
            TASK_PRIO_5,
            TASK_PRIO_6,
            TASK_PRIO_7,
            TASK_PRIO_8,
            TASK_PRIO_9
        };

        static void CreateTask(task_func func, std::uintptr_t arg, enum PriorityLevel priority, const char* name);
        static void DestroyTask();
        static void Sleep(std::uint32_t ticks);
        static void StartOS();
    };
}

#endif // OS_H_