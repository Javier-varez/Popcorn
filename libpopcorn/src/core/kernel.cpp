/*
 * This file is part of the Popcorn
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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <cstddef>
#include <cstring>

#include "popcorn/core/kernel.h"
#include "popcorn/core/cortex-m_port.h"
#include "popcorn/core/syscall_idx.h"
#include "popcorn/core/lockable.h"

#include "popcorn/utils/memory_management.h"

#include "popcorn/primitives/critical_section.h"

using std::uint8_t;
using std::uint32_t;
using std::uint64_t;
using std::uintptr_t;

namespace Popcorn {
Kernel* g_kernel = nullptr;

uint8_t* Kernel::AllocateTaskStack(Popcorn::task_control_block *tcb,
                                   size_t size) const {
  uint8_t* task_stack_top = nullptr;

  uint8_t* stack = reinterpret_cast<uint8_t*>(OsMalloc(size));
  if (stack != nullptr) {
    tcb->stack_base = reinterpret_cast<uintptr_t>(&stack[0]);
    task_stack_top = reinterpret_cast<uint8_t*>(&stack[size]);
  }

  return task_stack_top;
}

void Kernel::CreateTask(task_func func,
                            void* arg,
                            Priority priority,
                            const char* name,
                            uint32_t stack_size) {
  task_control_block* tcb = reinterpret_cast<task_control_block*>(
    OsMalloc(sizeof(task_control_block)));
  if (nullptr == tcb) {
    return;
  }

  stack_size = stack_size < MINIMUM_TASK_STACK_SIZE ?
               MINIMUM_TASK_STACK_SIZE : stack_size;

  uint8_t* task_stack_top = AllocateTaskStack(tcb, stack_size);
  if (task_stack_top == nullptr) {
    OsFree(tcb);
    return;
  }

  uint8_t* task_stack = m_mcu->InitializeTask(task_stack_top, func, arg);

  auto task_stack_ptr = reinterpret_cast<uintptr_t>(task_stack);
  tcb->stack_ptr = task_stack_ptr;
  tcb->arg = reinterpret_cast<uintptr_t>(arg);
  tcb->priority = priority;
  tcb->base_priority = priority;
  tcb->func = func;
  tcb->state = task_state::READY;
  tcb->run_last_timestamp = 0;  // never
  strncpy(tcb->name, name, MAX_TASK_NAME);
  LinkedList_AddEntry(m_ready_list, tcb, list);
}

void IdleTask(void *arg) {
  (void)arg;
  while (1) { }
}

void Kernel::StartOS() {
  // The idle task will run whenever there is no other
  // good candidate to run. It has the lowest priority.
  CreateTask(IdleTask,
             nullptr,
             Priority::IDLE,
             "Idle",
             MINIMUM_TASK_STACK_SIZE);

  // Configure interrupts and priorities.
  m_mcu->Initialize();
  // Trigger a context change to schedule the first task.
  m_mcu->TriggerPendSV();
}

uint64_t Kernel::GetTicks() {
  CriticalSection s;
  return m_ticks;
}

void Kernel::Sleep(uint32_t num_ticks) {
  task_control_block* tcb = m_current_task;
  tcb->blockArgument.timestamp = num_ticks + GetTicks();

  // Send task to sleep
  tcb->state = task_state::SLEEPING;
  LinkedList_RemoveEntry(m_ready_list, tcb, list);
  LinkedList_AddEntry(m_sleeping_list, tcb, list);

  m_mcu->TriggerPendSV();
}

void Kernel::DestroyTask() {
  if (!m_current_task) {
    return;
  }

  task_control_block *tcb = m_current_task;
  // Remove task from task_list and free space
  LinkedList_RemoveEntry(m_ready_list, tcb, list);
  OsFree(reinterpret_cast<void*>(tcb->stack_base));
  OsFree(tcb);

  m_current_task = nullptr;

  // Scheduler needs to select another task to run
  m_mcu->TriggerPendSV();
}

void Kernel::Yield() {
  // Scheduler needs to run and check which task to run
  m_mcu->TriggerPendSV();
}

void Kernel::Wait(const Lockable& lockable) {
  task_control_block* tcb = m_current_task;

  // Send task to the blocked state
  tcb->state = task_state::BLOCKED;
  tcb->blockArgument.lockable = &lockable;

  auto *blocker_task = lockable.GetBlockerTask();
  ATE_ASSERT(nullptr != blocker_task);
  if (blocker_task->priority < m_current_task->priority) {
    /* Inherit priority */
    blocker_task->priority = m_current_task->priority;
  }

  // Take the current task from the ready list and move it to the
  // blocked list
  LinkedList_RemoveEntry(m_ready_list, tcb, list);
  LinkedList_AddEntry(m_blocked_list, tcb, list);

  // Scheduler needs to select another task to run as
  // priorities may have changed and the current task is not
  // in a runnable state anymore.
  m_mcu->TriggerPendSV();
}

void Kernel::Lock(Lockable& lockable, bool acquired) {
  if (acquired) {
    lockable.SetBlockerTask(m_current_task);
  } else {
    // Restore original priority of the blocker
    auto *blocker_task = lockable.GetBlockerTask();
    ATE_ASSERT(nullptr != blocker_task);
    blocker_task->priority = blocker_task->base_priority;
    lockable.SetBlockerTask(nullptr);

    // Bring back blocked tasks by this resource
    task_control_block *tcb = nullptr;
    task_control_block *tcb_next = nullptr;

    LinkedList_WalkEntry_Safe(m_blocked_list, tcb, tcb_next, list) {
      // Walk through the list and make sure that all the
      // tasks that were blocked by this resource are now
      // ready to run.
      // This means removing them from the blocked list and
      // adding them back to the ready list.
      //
      // Using LinkedList_WalkEntry_Safe because we are removing
      // elements of the list inside the loop
      if (tcb->blockArgument.lockable == &lockable) {
        LinkedList_RemoveEntry(m_blocked_list, tcb, list);
        LinkedList_AddEntry(m_ready_list, tcb, list);
        tcb->state = task_state::READY;
      }
    }

    m_mcu->TriggerPendSV();
  }
}

/**
 * @todo Implement this method to report the failures
 */
void Kernel::RegisterError() { }

__WEAK void Kernel::TriggerSchedulerEntryHook() { }

__WEAK void Kernel::TriggerSchedulerExitHook() { }

void Kernel::TriggerScheduler() {
  TriggerSchedulerEntryHook();
  uint64_t num_ticks = GetTicks();

  if (m_current_task &&
      m_current_task->state == task_state::RUNNING) {
    m_current_task->state = task_state::READY;
    m_current_task->run_last_timestamp = num_ticks;
  }

  // Select next task based on priority
  m_current_task = nullptr;
  task_control_block* tcb = nullptr;
  LinkedList_WalkEntry(m_ready_list, tcb, list) {
    if (tcb->state == task_state::READY) {
      if (!m_current_task || (tcb->priority > m_current_task->priority)) {
        m_current_task = tcb;
      } else if ((tcb->priority == m_current_task->priority) &&
                 (tcb->run_last_timestamp < m_current_task->run_last_timestamp)) {
        m_current_task = tcb;
      }
    }
  }

  ATE_ASSERT(m_current_task != nullptr);
  m_current_task->state = task_state::RUNNING;

  TriggerSchedulerExitHook();
}

void Kernel::TriggerScheduler_Static() {
  g_kernel->TriggerScheduler();
}

void Kernel::CheckTaskNeedsAwakening() {
  task_control_block* tcb = nullptr;
  task_control_block* next_tcb = nullptr;

  LinkedList_WalkEntry_Safe(m_sleeping_list, tcb, next_tcb, list) {
    // Resume task if asleep
    if (m_ticks >= tcb->blockArgument.timestamp) {
      tcb->state = task_state::READY;
      LinkedList_RemoveEntry(m_sleeping_list, tcb, list);
      LinkedList_AddEntry(m_ready_list, tcb, list);
    }
  }
}

void Kernel::HandleTick() {
  {
    CriticalSection s;
    m_ticks++;
  }
  CheckTaskNeedsAwakening();
  m_mcu->TriggerPendSV();
}

Kernel::Kernel(Hw::MCU* mcu) :
    m_mcu(mcu) {
  m_mcu->RegisterSyscallImpl(this);
  g_kernel = this;
}

}  // namespace Popcorn

__WEAK void App_SysTick_Hook() { }

CLINKAGE void SysTick_Handler() {
  App_SysTick_Hook();
  if (Popcorn::g_kernel) {
    Popcorn::g_kernel->HandleTick();
  }
}
