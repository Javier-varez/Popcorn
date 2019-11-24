#include "stdint.h"

#define MAX_TASKS 2
#define STACK_SIZE 10241
#define TASK_STACK_SIZE 1024

typedef void (*task_func)();

void CreateTask(task_func func);
void StartOS();