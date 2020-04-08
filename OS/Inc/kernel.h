/* 
 * This file is part of the Cortex-M Scheduler
 * Copyright (c) 2020 Javier Alvarez
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OS_INC_KERNEL_H_
#define OS_INC_KERNEL_H_

#include "Inc/platform.h"
#include "Inc/blockable.h"
#include "Inc/linked_list.h"
#include "Inc/os_config.h"

// Forward declaration of classes and functions
namespace Hw {
class MCU;
}
class KernelTest;

CLINKAGE void SysTick_Handler();
CLINKAGE void PendSV_Handler();

#define EXC_RETURN_PSP_UNPRIV       (0xFFFFFFFD)
#define XPSR_INIT_VALUE             (1U << 24)

namespace OS {
enum class task_state {
    READY,
    RUNNING,
    SLEEPING,
    BLOCKED
};

union block_argument {
    std::uint64_t timestamp;
    const OS::Blockable* blockable;
};

struct task_control_block {
    // Assembly code relies on the stack_ptr being at
    // offset 0 from the task_control block
    // DO NOT CHANGE!
    std::uintptr_t                       stack_ptr;
    std::uintptr_t                       arg;
    task_func                            func;
    uintptr_t                            stack_base;
    LinkedList_t                         list;
    enum OS::Priority                    priority;
    enum OS::Priority                    base_priority;
    enum task_state                      state;
    char                                 name[MAX_TASK_NAME];
    block_argument                       blockArgument;
    std::uint64_t                        run_last_timestamp;
};

struct auto_task_stack_frame {
    std::uint32_t r0;
    std::uint32_t r1;
    std::uint32_t r2;
    std::uint32_t r3;
    std::uint32_t r12;
    std::uint32_t lr;
    std::uint32_t pc;
    std::uint32_t xpsr;
};

struct manual_task_stack_frame {
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

struct task_stack_frame {
    struct manual_task_stack_frame manualsave;
    struct auto_task_stack_frame autosave;
};

class Kernel {
 public:
    Kernel();

    TEST_VIRTUAL void StartOS();
    TEST_VIRTUAL void CreateTask(
        task_func func,
        uintptr_t arg,
        OS::Priority priority,
        const char* name,
        std::uint32_t stack_size);
    TEST_VIRTUAL void Sleep(std::uint32_t ticks);
    TEST_VIRTUAL void DestroyTask();
    TEST_VIRTUAL void Yield();
    TEST_VIRTUAL void Wait(const Blockable* blockable);
    TEST_VIRTUAL void RegisterError(struct auto_task_stack_frame* args);
    TEST_VIRTUAL void Lock(Blockable* blockable, bool acquired);

    TEST_VIRTUAL std::uint64_t GetTicks();
    TEST_VIRTUAL ~Kernel() = default;

    TEST_VIRTUAL task_control_block* GetCurrentTask() {
        return current_task;
    }

 private:
    TEST_VIRTUAL void TriggerScheduler();
    TEST_VIRTUAL void HandleTick();
    TEST_VIRTUAL void TriggerSchedulerEntryHook();
    TEST_VIRTUAL void TriggerSchedulerExitHook();
    TEST_VIRTUAL void HandleErrorHook(
        uint32_t pc,
        uint32_t r0,
        uint32_t r1,
        uint32_t r2,
        uint32_t r3,
        uint32_t sp);
    TEST_VIRTUAL void CheckTaskNeedsAwakening();

    static void TriggerScheduler_Static();

    struct task_control_block*  current_task  = nullptr;
    LinkedList_t*               ready_list    = nullptr;
    LinkedList_t*               blocked_list  = nullptr;
    LinkedList_t*               sleeping_list = nullptr;
    std::uint64_t               ticks         = 0;

    friend void ::SysTick_Handler();
    friend void ::PendSV_Handler();
    friend class ::KernelTest;
};
extern Kernel *g_kernel;
}  // namespace OS

#endif  // OS_INC_KERNEL_H_
