/*
 * This file is part of Popcorn
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

#ifndef POPCORN_CORE_KERNEL_H_
#define POPCORN_CORE_KERNEL_H_

#include "popcorn/API/syscall.h"
#include "popcorn/core/lockable.h"
#include "popcorn/utils/linked_list.h"
#include "popcorn/platform.h"
#include "popcorn/os_config.h"

// Forward declaration of classes and functions
namespace Hw {
class MCU;
}
class KernelTest;

CLINKAGE void SysTick_Handler();
CLINKAGE void PendSV_Handler();

namespace Popcorn {
enum class task_state {
  READY,
  RUNNING,
  SLEEPING,
  BLOCKED
};

union block_argument {
  std::uint64_t timestamp;
  const Popcorn::Lockable* lockable;
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
  Popcorn::Priority                         priority;
  Popcorn::Priority                         base_priority;
  task_state                           state;
  char                                 name[MAX_TASK_NAME];
  block_argument                       blockArgument;
  std::uint64_t                        run_last_timestamp;
};

class Kernel : public ISyscall {
 public:
  explicit Kernel(Hw::MCU* mcu);

  void StartOS() override;
  void CreateTask(task_func func, void* arg, Popcorn::Priority priority,
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

  TEST_VIRTUAL std::uint8_t* AllocateTaskStack(Popcorn::task_control_block *tcb,
                                               std::size_t size) const;

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

}  // namespace Popcorn

#endif  // POPCORN_CORE_KERNEL_H_
