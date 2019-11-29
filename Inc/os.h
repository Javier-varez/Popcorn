#include "stdint.h"

#define MAX_TASKS           2
#define POOL_SIZE          (1024 * 10)
#define TASK_STACK_SIZE     1024

typedef void (*task_func)();

void CreateTask(task_func func);
void StartOS();