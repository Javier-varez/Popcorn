#include "stdint.h"

#define MAX_TASKS           2
#define POOL_SIZE           (1024 * 10)
#define TASK_STACK_SIZE     1024
#define MAX_TASK_NAME       10

typedef void (*task_func)();

struct task_control_block
{
    uintptr_t stack_ptr;
    uintptr_t arg;
    task_func func;
    char name[MAX_TASK_NAME];
} __PACKED;

void CreateTask(task_func func, uintptr_t arg, char* name);
void StartOS();