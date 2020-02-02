#ifndef KERNEL_H_
#define KERNEL_H_

#include "platform.h"
#include "blockable.h"
#include "linked_list.h"
#include "os_config.h"

// Forward declaration of classes and functions
namespace Hw
{
    class MCU;
}
class KernelTest;

CLINKAGE void SysTick_Handler();
CLINKAGE void PendSV_Handler();

#define EXC_RETURN_PSP_UNPRIV       (0xFFFFFFFD)
#define XPSR_INIT_VALUE             (1U << 24)

namespace OS
{
    enum class task_state
    {
        RUNNABLE,
        RUNNING,
        SLEEPING,
        BLOCKED
    };

    union block_argument
    {
        std::uint64_t timestamp;
        const OS::Blockable* blockable;
    };

    struct task_control_block
    {
        // Assembly code relies on the stack_ptr being at
        // offset 0 from the task_control block
        // DO NOT CHANGE!
        uintptr_t                            stack_ptr;
        uintptr_t                            arg;
        task_func                            func;
        uintptr_t                            stack_base;
        LinkedList_t                         list;
        enum OS::Priority                    priority;
        enum task_state                      state;
        char                                 name[MAX_TASK_NAME];
        block_argument                       blockArgument;
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

    class Kernel {
    public:
        Kernel();

        TEST_VIRTUAL void StartOS();
        TEST_VIRTUAL void CreateTask(task_func func, uintptr_t arg, enum OS::Priority priority, const char* name);
        TEST_VIRTUAL void Sleep(std::uint32_t ticks);
        TEST_VIRTUAL void DestroyTask();
        TEST_VIRTUAL void Yield();
        TEST_VIRTUAL void Wait(const Blockable* blockable);
        TEST_VIRTUAL void RegisterError(struct auto_task_stack_frame* args);

        TEST_VIRTUAL std::uint64_t GetTicks();

    private:
        TEST_VIRTUAL void TriggerScheduler();
        TEST_VIRTUAL void HandleTick();

        static void TriggerScheduler_Static();

        struct task_control_block*  current_task = nullptr;
        LinkedList_t*               task_list    = nullptr;
        std::uint64_t               ticks        = 0;

        friend void ::SysTick_Handler();
        friend void ::PendSV_Handler();
        friend class ::KernelTest;
    };
    extern Kernel *g_kernel;
}

#endif