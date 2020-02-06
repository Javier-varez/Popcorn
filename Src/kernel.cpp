#include <cstddef>
#include <cstring>

#include "kernel.h"
#include "cortex-m_port.h"
#include "memory_management.h"
#include "critical_section.h"
#include "syscall_idx.h"

namespace OS
{
    Kernel* g_kernel = nullptr;
}

// Check the minimum task stack size is larger than the stack frame + alignment
static_assert(MINIMUM_TASK_STACK_SIZE > (sizeof(OS::task_stack_frame) + sizeof(uint32_t)));

static void* AllocateTaskStack(struct OS::task_control_block *tcb, size_t size)
{
    void* task_stack_top = NULL;

    std::uint8_t* stack = reinterpret_cast<std::uint8_t*>(OsMalloc(size));
    if (stack != NULL)
    {
        tcb->stack_base = (uintptr_t)&stack[0];
        task_stack_top = &stack[size];
    }

    return task_stack_top;
}

static std::uint8_t* AllocateTaskStackFrame(std::uint8_t* stack_ptr)
{
    if (stack_ptr == NULL)
        return NULL;

    uintptr_t retval =
        (uintptr_t)(stack_ptr - sizeof(struct OS::auto_task_stack_frame));

    // Align on 8 byte boundary
    return reinterpret_cast<std::uint8_t*>(retval & 0xfffffff8);
}

static struct OS::task_stack_frame* AllocateCompleteTaskStackFrame(std::uint8_t* stack_ptr)
{
    std::uint8_t* ptr = AllocateTaskStackFrame(stack_ptr);
    return reinterpret_cast<struct OS::task_stack_frame*>(ptr - sizeof(struct OS::manual_task_stack_frame));
}

void DestroyTaskVeneer()
{
    OS::Syscall::Instance().DestroyTask();
}

static void InitializeTask(
    struct OS::task_stack_frame* stack_frame_ptr,
    task_func func,
    uintptr_t arg)
{
    stack_frame_ptr->autosave.pc = reinterpret_cast<std::uint32_t>(func);
    stack_frame_ptr->autosave.r0 = arg;
    stack_frame_ptr->autosave.xpsr = XPSR_INIT_VALUE;
    stack_frame_ptr->autosave.lr = (std::uint32_t)DestroyTaskVeneer; // Task return should destroy itself
    stack_frame_ptr->manualsave.lr = EXC_RETURN_PSP_UNPRIV;
}

void OS::Kernel::CreateTask(task_func func, uintptr_t arg, enum OS::Priority priority, const char* name, std::uint32_t stack_size)
{
    struct task_control_block* tcb = 
        (struct task_control_block*)OsMalloc(sizeof(struct task_control_block));
    if (NULL == tcb)
        return;

    stack_size = stack_size < MINIMUM_TASK_STACK_SIZE ? MINIMUM_TASK_STACK_SIZE : stack_size;

    void* task_stack_top = AllocateTaskStack(tcb, stack_size);
    if (task_stack_top == NULL)
    {
        OsFree(tcb);
        return;
    }

    struct OS::task_stack_frame* task_stack_ptr =
        AllocateCompleteTaskStackFrame(reinterpret_cast<std::uint8_t*>(task_stack_top));
    if (NULL == task_stack_ptr)
        return;    

    InitializeTask(task_stack_ptr, func, arg);

    tcb->stack_ptr = (uintptr_t)task_stack_ptr;
    tcb->arg  = arg;
    tcb->priority = priority;
    tcb->func = func;
    tcb->state = task_state::RUNNABLE;
    strncpy(tcb->name, name, MAX_TASK_NAME);
    LinkedList_AddEntry(task_list, tcb, list);
}

void IdleTask(void *arg)
{
    (void)arg;
    while(1);
}

void OS::Kernel::StartOS()
{
    // Create Idle Task
    CreateTask(IdleTask, (uintptr_t)NULL, OS::Priority::IDLE, "Idle", MINIMUM_TASK_STACK_SIZE);

    Hw::g_mcu->Initialize();
    Hw::g_mcu->TriggerPendSV();
}

std::uint64_t OS::Kernel::GetTicks()
{
    OS::CriticalSection s;
    return ticks;
}

void OS::Kernel::Sleep(std::uint32_t ticks)
{
    struct task_control_block* tcb = current_task;
    tcb->blockArgument.timestamp = ticks + GetTicks();

    // Send task to sleep
    tcb->state = task_state::SLEEPING;

    Hw::g_mcu->TriggerPendSV();
}

void OS::Kernel::DestroyTask()
{
    if (!current_task)
        return;

    struct task_control_block *tcb = current_task;
    // Remove task from task_list and free space
    LinkedList_RemoveEntry(task_list, tcb, list);
    OsFree(reinterpret_cast<void*>(tcb->stack_base));
    OsFree(tcb);

    current_task = NULL;

    Hw::g_mcu->TriggerPendSV();
}

void OS::Kernel::Yield()
{
    Hw::g_mcu->TriggerPendSV();
}

void OS::Kernel::Wait(const OS::Blockable* blockable)
{
    struct task_control_block* tcb = current_task;

    // Send task to the blocked state
    tcb->state = task_state::BLOCKED;
    tcb->blockArgument.blockable = blockable;

    Hw::g_mcu->TriggerPendSV();
}

void OS::Kernel::RegisterError(struct OS::auto_task_stack_frame* args)
{
    // TODO :: Obtain info and report error
}

void OS::Kernel::TriggerScheduler()
{
    if (current_task &&
        current_task->state == task_state::RUNNING)
        current_task->state = task_state::RUNNABLE;

    uint64_t ticks = GetTicks();

    // Select next task based on priority
    current_task = NULL;
    struct task_control_block* tcb = NULL;
    LinkedList_WalkEntry(task_list, tcb, list)
    {
        // Resume task if asleep/blocked
        if ((tcb->state == task_state::SLEEPING) &&
            (ticks >= tcb->blockArgument.timestamp))
        {
            tcb->state = task_state::RUNNABLE;
        }
        else if ((tcb->state == task_state::BLOCKED) &&
                 !tcb->blockArgument.blockable->IsBlocked())
        {
            tcb->state = task_state::RUNNABLE;
        }

        if ((tcb->state == task_state::RUNNABLE) && 
            (current_task == NULL || (tcb->priority > current_task->priority)))
        {
            current_task = tcb;
        }
    }
    current_task->state = task_state::RUNNING;
}

void OS::Kernel::TriggerScheduler_Static()
{
    g_kernel->TriggerScheduler();
}

void OS::Kernel::HandleTick()
{
    OS::CriticalSection s;
    ticks++;
    Hw::g_mcu->TriggerPendSV();
}

OS::Kernel::Kernel()
{
    OS::g_kernel = this;
}

__attribute__((weak)) void App_SysTick_Hook()
{

}

CLINKAGE void SysTick_Handler()
{
    App_SysTick_Hook();
    if (OS::g_kernel)
    {
        OS::g_kernel->HandleTick();
    }
}

