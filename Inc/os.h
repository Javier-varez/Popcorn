#include "stdint.h"

#define TASK_STACK_SIZE     256
#define MAX_TASK_NAME       10

typedef void (*task_func)();

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

void CreateTask(task_func func, uintptr_t arg, enum PriorityLevel priority, char* name);
void Sleep(uint32_t ticks);
void StartOS();