#include "os.h"
#include "stm32f1xx.h"

#define __NAKED				__attribute__((naked))

#define STRINGIZE(_x)		#_x
#define SVC_CALL(_x)		asm volatile("svc " STRINGIZE(_x))

#define START_OS_SVC		2

#define EXC_RETURN_PSP_UNPRIV		0xFFFFFFFD

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
	uint32_t pc;
	uint32_t xpsr;
} __PACKED;

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
} __PACKED;

void CreateTask(task_func func)
{
	if (n_tasks < MAX_TASKS)
	{
		n_tasks++;
		struct complete_task_stack_frame* task_psp = 
			(struct complete_task_stack_frame*) &stack[STACK_SIZE - 1 - TASK_STACK_SIZE * n_tasks - sizeof(struct complete_task_stack_frame)];
		task_psp->autosave.pc = (uint32_t)func;
		task_psp->autosave.xpsr = 1 << 24;
		task_psp->autosave.lr = 0xA5A5A5A5;
        task_psp->lr = 0xfffffffd;
		tasks_psp[n_tasks - 1] = task_psp;
	}
}

static uint32_t StartOS_SVC_Handler()
{
	scheduler_active = 1;
    __NVIC_SetPriority(SysTick_IRQn, 0x00);
    __NVIC_SetPriority(PendSV_IRQn,  0xFF);

    struct complete_task_stack_frame* frame =
        (struct complete_task_stack_frame*)tasks_psp[0];

	// Start executing first task
	__set_PSP((uint32_t)&frame->autosave);
	__set_CONTROL(0x03);

	return EXC_RETURN_PSP_UNPRIV;
}

__NAKED void PendSV_Handler()
{
	asm volatile (
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
		"pxTasksPSP: .word tasks_psp         "
	);
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

void SVC_Handler_C(struct task_stack_frame* args)
{
	char* pc = (char*)args->pc;
	uint8_t svc_code = pc[-2]; // First byte of svc instruction
	uint32_t lr = 0;

	switch (svc_code)
	{
	case START_OS_SVC:
		lr = StartOS_SVC_Handler();
		break;
	
	default:
		while (1);
		break;
	}

	// Update lr if required
	asm volatile(
		"    teq %[lr], #0			\n"
		"    it ne								\n"
		"    movne lr, %[lr]			  "
		: : [lr] "r" (lr) 
	);
}

void StartOS()
{
	SVC_CALL(START_OS_SVC);
}