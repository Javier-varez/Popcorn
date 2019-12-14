#include "os.h"
#include "stm32f1xx_hal.h"
#include <stddef.h>
#include <string.h>
#include "memory_management.h"
#include "linked_list.h"

#define __NAKED						__attribute__((naked))

#define __STRINGIZE(_x)				#_x
#define SVC_CALL(_x)				asm volatile("svc " __STRINGIZE(_x))

#define START_OS_SVC				(0)
#define CREATE_TASK_SVC				(1)
#define SLEEP_UNTIL_SVC				(2)

#define XPSR_INIT_VALUE				(1 << 24)
#define INVALID_LR_VALUE			(0xA5A5A5A5)
#define EXC_RETURN_PSP_UNPRIV		(0xFFFFFFFD)

enum task_state
{
	TASK_RUNNABLE,
	TASK_RUNNING,
	TASK_SLEEP,
};

struct task_control_block
{
    uintptr_t                   stack_ptr;
    uintptr_t                   arg;
    task_func                   func;
	LinkedList_t				list;
    enum PriorityLevel          priority;
	enum task_state             state;
    char                        name[MAX_TASK_NAME];
};

struct task_event
{
	struct task_control_block* task;
	uint32_t requested_wakeup_timestamp;
	LinkedList_t list;
};

struct Scheduler
{
	struct task_control_block *current_task;
	LinkedList_t* task_list;
	LinkedList_t* event_list;
};

static struct Scheduler scheduler = 
{
	.current_task = NULL,
	.task_list = NULL,
	.event_list = NULL
};

struct auto_task_stack_frame
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t xpsr;
};

struct manual_task_stack_frame
{
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t lr;
};

struct task_stack_frame
{
		struct manual_task_stack_frame manualsave;
        struct auto_task_stack_frame autosave;
};

static void* AllocateTaskStack(size_t size)
{
	void* task_stack_top = NULL;

	uint8_t* stack = OsMalloc(size);
	if (stack != NULL)
	{
		task_stack_top = &stack[size-1];
	}

	return task_stack_top;
}

static void* AllocateTaskStackFrame(void* stack_ptr)
{
	if (stack_ptr == NULL)
		return NULL;

	uintptr_t retval =
		(uintptr_t)(stack_ptr - sizeof(struct auto_task_stack_frame));

	// Align on 8 byte boundary
	return (void*)(retval & 0xfffffff8);
}

static void* AllocateCompleteTaskStackFrame(void* stack_ptr)
{
	void* ptr = AllocateTaskStackFrame(stack_ptr);
	return ptr - sizeof(struct manual_task_stack_frame);
}

static void InitializeTask(
	struct task_stack_frame* stack_frame_ptr,
	task_func func,
	uintptr_t arg)
{
	stack_frame_ptr->autosave.pc = (uint32_t)func;
	stack_frame_ptr->autosave.r0 = arg;
	stack_frame_ptr->autosave.xpsr = XPSR_INIT_VALUE;
	stack_frame_ptr->autosave.lr = INVALID_LR_VALUE;
	stack_frame_ptr->manualsave.lr = EXC_RETURN_PSP_UNPRIV;
}

void CreateTask_SVC_Handler(task_func func, uintptr_t arg, enum PriorityLevel priority, char* name)
{
	struct task_control_block* tcb = 
		(struct task_control_block*)OsMalloc(sizeof(struct task_control_block));
	if (NULL == tcb)
		return;

	void* task_stack_top = AllocateTaskStack(TASK_STACK_SIZE);
	if (task_stack_top == NULL)
	{
		OsFree(tcb);
		return;
	}

	struct task_stack_frame* task_stack_ptr =
		AllocateCompleteTaskStackFrame(task_stack_top);
	if (NULL == task_stack_ptr)
		return;	

	InitializeTask(task_stack_ptr, func, arg);

	tcb->stack_ptr = (uintptr_t)task_stack_ptr;
	tcb->arg  = arg;
	tcb->priority = priority;
	tcb->func = func;
	tcb->state = TASK_RUNNABLE;
	strncpy(tcb->name, name, MAX_TASK_NAME);
	LinkedList_AddEntry(scheduler.task_list, tcb, list);

	return;
}

static void IdleTask(void *arg)
{
	(void)arg;
	while(1);
}

__NAKED static uint32_t StartOS_SVC_Handler()
{
	// Create Idle Task
	CreateTask_SVC_Handler(IdleTask, (uintptr_t)NULL, TASK_PRIO_IDLE, "Idle");

	// Set first task	
	scheduler.current_task = 
		CONTAINER_OF(scheduler.task_list, struct task_control_block, list);
	scheduler.current_task->state = TASK_RUNNING;

	// Set OS IRQ priorities
    __NVIC_SetPriority(SysTick_IRQn, 0xFF); // Minimum priority for SysTick
    __NVIC_SetPriority(PendSV_IRQn,  0xFF); // Minimum priority for PendSV
    __NVIC_SetPriority(SVCall_IRQn,  0x00); // Maximum priority for SVC

	// Obtain first task stack pointer
    struct task_stack_frame* frame =
        (struct task_stack_frame*)scheduler.current_task->stack_ptr;

	// Start executing first task by setting correct stack pointer
	__set_PSP((uint32_t)&frame->autosave);

	// Obain top of the stack from current NVIC table and set MSP
	uintptr_t vtor = SCB->VTOR;
	uintptr_t stack_top = ((volatile uint32_t*)vtor)[0];
	__set_MSP(stack_top);
	__DSB();
    __ISB();

	// Use exception return code for Unprivileged mode and PSP stack
	asm volatile (
		"    mov lr, %[exc_return]	\n"
		"    bx lr      				\n"
		: : [exc_return] "i" (EXC_RETURN_PSP_UNPRIV)
	);
}

static uint32_t GetTicks()
{
	return HAL_GetTick();
}

void Sleep_SVC_Handler(uint32_t ticks)
{
	struct task_event* event = (struct task_event*) OsMalloc(sizeof(struct task_event));
	
	struct task_control_block* tcb = scheduler.current_task;
	event->requested_wakeup_timestamp = ticks + GetTicks();
	event->task = tcb;

	// Add event to list
	LinkedList_AddEntry(scheduler.event_list, event, list);

    // Send task to sleep
	tcb->state = TASK_SLEEP;

	// Trigger scheduler (PendSV call)
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void SchedulerTrigger()
{
	if (scheduler.current_task->state == TASK_RUNNING)
		scheduler.current_task->state = TASK_RUNNABLE;

	uint32_t ticks = GetTicks();
	// Check Events
	struct task_event *event;
	struct task_event *next_event;
	LinkedList_WalkEntry_Safe(scheduler.event_list, event, next_event, list)
	{
		if (ticks >= event->requested_wakeup_timestamp)
		{
			event->task->state = TASK_RUNNABLE;
			LinkedList_RemoveEntry(scheduler.event_list, event, list);
			OsFree(event);
		}
	}

	// Select next task based on priority
	scheduler.current_task = NULL;
	struct task_control_block* tcb = NULL;
	LinkedList_WalkEntry(scheduler.task_list, tcb, list)
	{
		if ((tcb->state == TASK_RUNNABLE) && 
			(scheduler.current_task == NULL || (tcb->priority > scheduler.current_task->priority)))
		{
			scheduler.current_task = tcb;
		}
	}
	scheduler.current_task->state = TASK_RUNNING;
}

__NAKED void PendSV_Handler()
{
	asm volatile (
		"    mrs r0, psp                       \n"
		"    stmdb r0!, {r4-r11, r14}          \n"
		"    ldr r1, [%[current_task_ptr]]     \n"
		"    str r0, [r1]                      \n" // scheduler.current_task->stack_ptr = psp
		"    push {r0, %[current_task_ptr]}    \n"
		"    bl SchedulerTrigger		       \n"
		"    pop {r0,  %[current_task_ptr]}    \n"
		"    ldr r1, [%[current_task_ptr]]     \n"
		"    ldr r0, [r1]                      \n" 
		"    ldmia r0!, {r4-r11, r14}          \n"
		"    msr psp, r0                       \n" // psp = scheduler.current_task->stack_ptr
		"    isb                               \n"
		"    bx lr                             \n"
		: : 
		[current_task_ptr] "r" (&scheduler.current_task)
	);
}

void SysTick_Handler()
{
	HAL_IncTick();
	if (scheduler.current_task != NULL)
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

__NAKED void SVC_Handler()
{
	asm volatile (
		"    tst lr, #4              \n"
		"    ite eq                  \n"
		"    mrseq r0, msp           \n"
		"    mrsne r0, psp           \n"
		"    b SVC_Handler_C           "
	);
}

void SVC_Handler_C(struct auto_task_stack_frame* args)
{
	char* pc = (char*)args->pc;
	uint8_t svc_code = pc[-2]; // First byte of svc instruction

	switch (svc_code)
	{
	case START_OS_SVC:
		StartOS_SVC_Handler();
		break;
	
	case CREATE_TASK_SVC:
		CreateTask_SVC_Handler(
			(task_func)args->r0, 
			(uintptr_t)args->r1, 
			(enum PriorityLevel)args->r2, 
			(char*)args->r3);
		break;
	case SLEEP_UNTIL_SVC:
		Sleep_SVC_Handler((uint32_t)args->r0);
		break;

	default:
		while (1);
		break;
	}
}

void StartOS()
{
	SVC_CALL(START_OS_SVC);
}

void CreateTask(task_func func, uintptr_t arg, enum PriorityLevel priority, char* name)
{
	SVC_CALL(CREATE_TASK_SVC);
}

void Sleep(uint32_t ticks)
{
	SVC_CALL(SLEEP_UNTIL_SVC);
}