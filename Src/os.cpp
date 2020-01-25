#include "os.h"
#include "stm32f1xx_hal.h"
#include "memory_management.h"
#include "linked_list.h"
#include "spinlock.h"

#include <cstddef>
#include <cstring>

#define __NAKED                     __attribute__((naked))

#define __STRINGIZE(_x)             #_x
#define SVC_CALL(_x)                asm volatile("svc " __STRINGIZE(_x))

#define START_OS_SVC                (0)
#define CREATE_TASK_SVC             (1)
#define SLEEP_UNTIL_SVC             (2)
#define DESTROY_TASK_SVC            (3)

#define XPSR_INIT_VALUE             (1 << 24)
#define EXC_RETURN_PSP_UNPRIV       (0xFFFFFFFD)

enum task_state
{
    TASK_RUNNABLE,
    TASK_RUNNING,
    TASK_SLEEP,
};

struct task_control_block
{
    uintptr_t                            stack_ptr;
    uintptr_t                            arg;
    task_func                            func;
    uintptr_t                            stack_base;
    LinkedList_t                         list;
    enum OS::Scheduler::PriorityLevel    priority;
    enum task_state                      state;
    char                                 name[MAX_TASK_NAME];
};

struct task_event
{
    struct task_control_block*  task;
    std::uint32_t               requested_wakeup_timestamp;
    LinkedList_t                list;
};

struct Scheduler
{
    struct task_control_block*  current_task;
    LinkedList_t*               task_list;
    LinkedList_t*               event_list;
};

static struct Scheduler scheduler = 
{
    .current_task = NULL,
    .task_list = NULL,
    .event_list = NULL
};

struct auto_task_stack_frame
{
    std::uint32_t r0;
    std::uint32_t r1;
    std::uint32_t r2;
    std::uint32_t r3;
    std::uint32_t r12;
    std::uint32_t lr;
    std::uint32_t pc;
    std::uint32_t xpsr;
};

struct manual_task_stack_frame
{
    std::uint32_t r4;
    std::uint32_t r5;
    std::uint32_t r6;
    std::uint32_t r7;
    std::uint32_t r8;
    std::uint32_t r9;
    std::uint32_t r10;
    std::uint32_t r11;
    std::uint32_t lr;
};

struct task_stack_frame
{
    struct manual_task_stack_frame manualsave;
    struct auto_task_stack_frame autosave;
};

static void* AllocateTaskStack(struct task_control_block *tcb, size_t size)
{
    void* task_stack_top = NULL;

    std::uint8_t* stack = reinterpret_cast<std::uint8_t*>(OsMalloc(size));
    if (stack != NULL)
    {
        tcb->stack_base = (uintptr_t)&stack[0];
        task_stack_top = &stack[size-1];
    }

    return task_stack_top;
}

static std::uint8_t* AllocateTaskStackFrame(std::uint8_t* stack_ptr)
{
    if (stack_ptr == NULL)
        return NULL;

    uintptr_t retval =
        (uintptr_t)(stack_ptr - sizeof(struct auto_task_stack_frame));

    // Align on 8 byte boundary
    return reinterpret_cast<std::uint8_t*>(retval & 0xfffffff8);
}

static struct task_stack_frame* AllocateCompleteTaskStackFrame(std::uint8_t* stack_ptr)
{
    std::uint8_t* ptr = AllocateTaskStackFrame(stack_ptr);
    return reinterpret_cast<struct task_stack_frame*>(ptr - sizeof(struct manual_task_stack_frame));
}

static void InitializeTask(
    struct task_stack_frame* stack_frame_ptr,
    task_func func,
    uintptr_t arg)
{
    stack_frame_ptr->autosave.pc = reinterpret_cast<std::uint32_t>(func);
    stack_frame_ptr->autosave.r0 = arg;
    stack_frame_ptr->autosave.xpsr = XPSR_INIT_VALUE;
    stack_frame_ptr->autosave.lr = reinterpret_cast<std::uint32_t>(OS::Scheduler::DestroyTask); // Task return should destroy itself
    stack_frame_ptr->manualsave.lr = EXC_RETURN_PSP_UNPRIV;
}

void CreateTask_SVC_Handler(task_func func, uintptr_t arg, enum OS::Scheduler::PriorityLevel priority, const char* name)
{
    struct task_control_block* tcb = 
        (struct task_control_block*)OsMalloc(sizeof(struct task_control_block));
    if (NULL == tcb)
        return;

    void* task_stack_top = AllocateTaskStack(tcb, TASK_STACK_SIZE);
    if (task_stack_top == NULL)
    {
        OsFree(tcb);
        return;
    }

    struct task_stack_frame* task_stack_ptr =
        AllocateCompleteTaskStackFrame(reinterpret_cast<std::uint8_t*>(task_stack_top));
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

__NAKED static std::uint32_t StartOS_SVC_Handler()
{
    // Create Idle Task
    CreateTask_SVC_Handler(IdleTask, (uintptr_t)NULL, OS::Scheduler::TASK_PRIO_IDLE, "Idle");

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
    __set_PSP(reinterpret_cast<std::uint32_t>(&frame->autosave));

    // Obain top of the stack from current NVIC table and set MSP
    uintptr_t vtor = SCB->VTOR;
    uintptr_t stack_top = (reinterpret_cast<volatile std::uint32_t*>(vtor))[0];
    __set_MSP(stack_top);
    __DSB();
    __ISB();

    // Use exception return code for Unprivileged mode and PSP stack
    asm volatile (
        "    mov lr, %[exc_return]    \n"
        "    bx lr                    \n"
        : : [exc_return] "i" (EXC_RETURN_PSP_UNPRIV)
    );
}

static std::uint32_t GetTicks()
{
    return HAL_GetTick();
}

void Sleep_SVC_Handler(std::uint32_t ticks)
{
    struct task_event* event = reinterpret_cast<struct task_event*>(OsMalloc(sizeof(struct task_event)));
    
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

void DestroyTask_SVC_Handler()
{
    if (!scheduler.current_task)
        return;

    struct task_control_block *tcb = scheduler.current_task;
    // Remove task from task_list and free space
    LinkedList_RemoveEntry(scheduler.task_list, tcb, list);
    OsFree(reinterpret_cast<void*>(tcb->stack_base));
    OsFree(tcb);

    scheduler.current_task = NULL;
    // Trigger scheduler (PendSV)
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

extern "C"
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

extern "C"
__NAKED void PendSV_Handler()
{
    asm volatile (
        "               ldr r1, [%[current_task_ptr]]     \n"
        "               cbz r1, TaskSwitch                \n"
        "               mrs r0, psp                       \n"
        "               stmdb r0!, {r4-r11, r14}          \n"
        "               str r0, [r1]                      \n" // scheduler.current_task->stack_ptr = psp
        "TaskSwitch:    push {r0, %[current_task_ptr]}    \n"
        "               bl SchedulerTrigger               \n"
        "               pop {r0,  %[current_task_ptr]}    \n"
        "               ldr r1, [%[current_task_ptr]]     \n"
        "               ldr r0, [r1]                      \n"
        "               ldmia r0!, {r4-r11, r14}          \n"
        "               msr psp, r0                       \n" // psp = scheduler.current_task->stack_ptr
        "               isb                               \n"
        "               bx lr                             \n"
        : : 
        [current_task_ptr] "r" (&scheduler.current_task)
    );
}

extern "C"
void SysTick_Handler()
{
    HAL_IncTick();
    if (scheduler.current_task != NULL)
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

extern "C"
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

extern "C"
void SVC_Handler_C(struct auto_task_stack_frame* args)
{
    char* pc = (char*)args->pc;
    uint8_t svc_code = pc[-sizeof(uint16_t)]; // First byte of svc instruction

    switch (svc_code)
    {
    case START_OS_SVC:
        StartOS_SVC_Handler();
        break;
    
    case CREATE_TASK_SVC:
        CreateTask_SVC_Handler(
            (task_func)args->r0, 
            (uintptr_t)args->r1, 
            (enum OS::Scheduler::PriorityLevel)args->r2, 
            (char*)args->r3);
        break;
    case SLEEP_UNTIL_SVC:
        Sleep_SVC_Handler((uint32_t)args->r0);
        break;
    case DESTROY_TASK_SVC:
        DestroyTask_SVC_Handler();
        break;

    default:
        while (1);
        break;
    }
}

namespace OS {
    void Scheduler::StartOS()
    {
        SVC_CALL(START_OS_SVC);
    }

    void Scheduler::CreateTask(task_func func, std::uintptr_t arg, enum PriorityLevel priority, const char* name)
    {
        SVC_CALL(CREATE_TASK_SVC);
    }

    void Scheduler::DestroyTask()
    {
        SVC_CALL(DESTROY_TASK_SVC);
    }

    void Scheduler::Sleep(std::uint32_t ticks)
    {
        SVC_CALL(SLEEP_UNTIL_SVC);
    }
}
