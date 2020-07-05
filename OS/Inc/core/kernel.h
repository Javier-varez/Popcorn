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

#ifndef OS_INC_CORE_KERNEL_H_
#define OS_INC_CORE_KERNEL_H_

#include "Inc/API/syscall.h"
#include "Inc/core/lockable.h"
#include "Inc/utils/linked_list.h"
#include "Inc/platform.h"
#include "Inc/os_config.h"

// Forward declaration of classes and functions
namespace Hw {
class MCU;
}
class KernelTest;

CLINKAGE void SysTick_Handler();
CLINKAGE void PendSV_Handler();

#define EXC_RETURN_PSP_UNPRIV       (0xFFFFFFFDU)
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
  const OS::Lockable* lockable;
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
  OS::Priority                         priority;
  OS::Priority                         base_priority;
  task_state                           state;
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
  manual_task_stack_frame manualsave;
  auto_task_stack_frame autosave;
};

class Kernel : public ISyscall {
 public:
  explicit Kernel(Hw::MCU* mcu);

  void StartOS() override;
  void CreateTask(task_func func, void* arg, OS::Priority priority,
                  const char* name, std::uint32_t stack_size) override;
  void Sleep(std::uint32_t ticks) override;
  void DestroyTask() override;
  void Yield() override;
  void Wait(const Lockable& lockable) override;
  void RegisterError() override;
  void Lock(Lockable& lockable, bool acquired) override;

  TEST_VIRTUAL std::uint64_t GetTicks();
  TEST_VIRTUAL ~Kernel() = default;

  TEST_VIRTUAL task_control_block* GetCurrentTask() {
    return m_current_task;
  }

 private:
  TEST_VIRTUAL void TriggerScheduler();
  TEST_VIRTUAL void HandleTick();
  TEST_VIRTUAL void TriggerSchedulerEntryHook();
  TEST_VIRTUAL void TriggerSchedulerExitHook();

  TEST_VIRTUAL void CheckTaskNeedsAwakening();

  static void TriggerScheduler_Static();

  Hw::MCU*                    m_mcu           = nullptr;
  task_control_block*         m_current_task  = nullptr;
  LinkedList_t*               m_ready_list    = nullptr;
  LinkedList_t*               m_blocked_list  = nullptr;
  LinkedList_t*               m_sleeping_list = nullptr;

  /**
   * @todo Use atomic for m_ticks instead of a regular uint64_t variable
   */
  std::uint64_t               m_ticks         = 0;

  friend void ::SysTick_Handler();
  friend void ::PendSV_Handler();
  friend class ::KernelTest;
};

}  // namespace OS

#endif  // OS_INC_CORE_KERNEL_H_
