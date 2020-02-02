#include "kernel.h"
#include "cortex-m_port.h"

CLINKAGE __NAKED void PendSV_Handler()
{
    asm volatile (
        "               ldr r1, [%[current_task_ptr]]     \n"
        "               cbz r1, TaskSwitch                \n"
        "               mrs r0, psp                       \n"
        "               stmdb r0!, {r4-r11, r14}          \n"
        "               str r0, [r1]                      \n" // scheduler.current_task->stack_ptr = psp
        "TaskSwitch:    push {%[current_task_ptr]}        \n"
        "               blx %[TriggerScheduler]           \n"
        "               pop {%[current_task_ptr]}         \n"
        "               ldr r1, [%[current_task_ptr]]     \n"
        "               ldr r0, [r1]                      \n"
        "               ldmia r0!, {r4-r11, r14}          \n"
        "               msr psp, r0                       \n" // psp = scheduler.current_task->stack_ptr
        "               isb                               \n"
        "               mov lr, %[exc_return]             \n"
        "               bx lr                             \n"
        : : 
        [current_task_ptr] "r" (&OS::g_kernel->current_task),
        [exc_return] "i" (EXC_RETURN_PSP_UNPRIV),
        [TriggerScheduler] "r" (OS::Kernel::TriggerScheduler_Static)
    );
}

CLINKAGE __NAKED void SVC_Handler()
{
    asm volatile (
        "    tst lr, #4              \n"
        "    ite eq                  \n"
        "    mrseq r0, msp           \n"
        "    mrsne r0, psp           \n"
        "    bx %[svc_handler]       \n"
        : : [svc_handler] "r" (Hw::MCU::HandleSVC_Static)
    );
}
