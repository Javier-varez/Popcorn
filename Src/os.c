#include "os.h"
#include "stm32f1xx.h"

volatile int scheduler_active = 0;

static volatile void* tasks_psp[MAX_TASKS];
static volatile int n_tasks = 0;
static volatile int current_task = 0;

static volatile uint8_t stack[STACK_SIZE] __attribute__ ((aligned(8)));

struct task_stack_frame
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t ret_addr;
	uint32_t xpsr;
} __attribute__((packed));

struct complete_task_stack_frame
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
       struct task_stack_frame autosave;
} __attribute__((packed));

void CreateTask(task_func func)
{
	if (n_tasks < MAX_TASKS)
	{
		n_tasks++;
		struct complete_task_stack_frame* task_psp = 
			(struct complete_task_stack_frame*) &stack[STACK_SIZE - 1 - TASK_STACK_SIZE * n_tasks - sizeof(struct complete_task_stack_frame)];
		task_psp->autosave.ret_addr = (uint32_t)func;
		task_psp->autosave.xpsr = 1 << 24;
		task_psp->autosave.lr = 0xA5A5A5A5;
        task_psp->lr = 0xfffffffd;
		tasks_psp[n_tasks - 1] = task_psp;
	}
}

void SVC_Handler()
{
	scheduler_active = 1;
    __NVIC_SetPriority(SysTick_IRQn, 0);
    __NVIC_SetPriority(PendSV_IRQn, 3);

    struct complete_task_stack_frame* frame =
        (struct complete_task_stack_frame*)tasks_psp[0];

	// Start executing first task
	__set_PSP((uint32_t)&frame->autosave);
	__set_CONTROL(0x03);
    asm volatile (
        "ldr lr, immediate_val\n"
        "bx lr\n"
        "immediate_val: .word 0xfffffffd"
    );
}

void PendSV_Handler()
{
	asm volatile(
		"    mrs r0, psp                   \n"
		"    isb                           \n"
		"    ldr r1, pxCurrentTask         \n"
		"    ldr r2, pxTasksPSP            \n"
		"    stmdb r0!, {r4-r11, r14}      \n"
		"    ldr r3, [r1]                  \n" // r3 = current_task
		"    str r0, [r2, r3, LSL #2]      \n" // tasks_psp[current_task] = psp
		"    eor r3, r3, #1                \n" // switch task
		"    str r3, [r1]                  \n" // current_task = (current_task + 1) % 2;
		"    ldr r0, [r2, r3, LSL #2]      \n" // r0 = tasks_psp[current_task]
		"    ldmia r0!, {r4-r11, r14}      \n"
		"    msr psp, r0                   \n"
		"    isb                           \n"
		"    bx lr                         \n"
		"pxCurrentTask: .word current_task \n"
		"pxTasksPSP: .word tasks_psp");
}

void StartOS()
{
    asm volatile("svc 0");
}