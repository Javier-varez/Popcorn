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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <cstddef>
#include <cstring>

#include "Inc/kernel.h"
#include "Inc/cortex-m_port.h"
#include "Inc/memory_management.h"
#include "Inc/critical_section.h"
#include "Inc/syscall_idx.h"

using std::uint8_t;
using std::uint32_t;
using std::uint64_t;
using std::uintptr_t;

using Hw::g_mcu;

namespace OS {
Kernel* g_kernel = nullptr;

// Check the minimum task stack size is larger than the stack frame + alignment
static_assert(MINIMUM_TASK_STACK_SIZE >
              (sizeof(task_stack_frame) + sizeof(uint32_t)));

static uint8_t* AllocateTaskStack(task_control_block *tcb,
                                       size_t size) {
  uint8_t* task_stack_top = nullptr;

  uint8_t* stack = reinterpret_cast<uint8_t*>(OsMalloc(size));
  if (stack != nullptr) {
    tcb->stack_base = reinterpret_cast<uintptr_t>(&stack[0]);
    task_stack_top = reinterpret_cast<uint8_t*>(&stack[size]);
  }

  return task_stack_top;
}

static uint8_t* AllocateTaskStackFrame(uint8_t* stack_ptr) {
  if (stack_ptr == nullptr) {
      return nullptr;
  }

  auto* stack_frame = stack_ptr - sizeof(auto_task_stack_frame);
  uintptr_t stack_frame_ptr = reinterpret_cast<uintptr_t>(stack_frame);

  // Align on 8 byte boundary
  return reinterpret_cast<uint8_t*>(stack_frame_ptr & 0xfffffff8);
}

static task_stack_frame*
AllocateCompleteTaskStackFrame(uint8_t* stack_ptr) {
  uint8_t* ptr = AllocateTaskStackFrame(stack_ptr);
  if (ptr == nullptr) {
    return nullptr;
  } else {
    ptr -= sizeof(manual_task_stack_frame);
    return reinterpret_cast<task_stack_frame*>(ptr);
  }
}

void DestroyTaskVeneer() {
  Syscall::Instance().DestroyTask();
}

static void InitializeTask(task_stack_frame* stack_frame_ptr,
                           task_func func,
                           void* arg) {
  auto lr = reinterpret_cast<uint32_t>(DestroyTaskVeneer);
  stack_frame_ptr->autosave.pc = reinterpret_cast<uint32_t>(func);
  stack_frame_ptr->autosave.r0 = reinterpret_cast<uintptr_t>(arg);
  stack_frame_ptr->autosave.xpsr = XPSR_INIT_VALUE;
  stack_frame_ptr->autosave.lr = lr;
  stack_frame_ptr->manualsave.lr = EXC_RETURN_PSP_UNPRIV;
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

  task_stack_frame* task_stack = AllocateCompleteTaskStackFrame(task_stack_top);
  if (nullptr == task_stack) {
    return;
  }

  InitializeTask(task_stack, func, arg);

  auto task_stack_ptr = reinterpret_cast<uintptr_t>(task_stack);
  tcb->stack_ptr = task_stack_ptr;
  tcb->arg = reinterpret_cast<uintptr_t>(arg);
  tcb->priority = priority;
  tcb->base_priority = priority;
  tcb->func = func;
  tcb->state = task_state::READY;
  tcb->run_last_timestamp = 0;  // never
  strncpy(tcb->name, name, MAX_TASK_NAME);
  LinkedList_AddEntry(ready_list, tcb, list);
}

void IdleTask(void *arg) {
  (void)arg;
  while (1) { }
}

void Kernel::StartOS() {
  // Create Idle Task
  CreateTask(IdleTask,
             0,
             Priority::IDLE,
             "Idle",
             MINIMUM_TASK_STACK_SIZE);

  g_mcu->Initialize();
  g_mcu->TriggerPendSV();
}

uint64_t Kernel::GetTicks() {
  CriticalSection s;
  return ticks;
}

void Kernel::Sleep(uint32_t num_ticks) {
  task_control_block* tcb = current_task;
  tcb->blockArgument.timestamp = num_ticks + GetTicks();

  // Send task to sleep
  tcb->state = task_state::SLEEPING;
  LinkedList_RemoveEntry(ready_list, tcb, list);
  LinkedList_AddEntry(sleeping_list, tcb, list);

  g_mcu->TriggerPendSV();
}

void Kernel::DestroyTask() {
  if (!current_task) {
    return;
  }

  task_control_block *tcb = current_task;
  // Remove task from task_list and free space
  LinkedList_RemoveEntry(ready_list, tcb, list);
  OsFree(reinterpret_cast<void*>(tcb->stack_base));
  OsFree(tcb);

  current_task = nullptr;

  g_mcu->TriggerPendSV();
}

void Kernel::Yield() {
  g_mcu->TriggerPendSV();
}

void Kernel::Wait(const Blockable* blockable) {
  task_control_block* tcb = current_task;

  // Send task to the blocked state
  tcb->state = task_state::BLOCKED;
  tcb->blockArgument.blockable = blockable;

  // Perform priority inheritance
  if (blockable->blocker->priority < current_task->priority) {
    blockable->blocker->priority = current_task->priority;
  }

  // Switch list
  LinkedList_RemoveEntry(ready_list, tcb, list);
  LinkedList_AddEntry(blocked_list, tcb, list);

  g_mcu->TriggerPendSV();
}

void Kernel::Lock(Blockable* blockable, bool acquired) {
  if (acquired) {
    blockable->blocker = current_task;
  } else {
    // Restore original priority of the blocker
    blockable->blocker->priority = blockable->blocker->base_priority;
    blockable->blocker = nullptr;
    // Bring back blocked tasks by this resource
    task_control_block *tcb = nullptr;
    task_control_block *tcb_next = nullptr;

    LinkedList_WalkEntry_Safe(blocked_list, tcb, tcb_next, list) {
      if (tcb->blockArgument.blockable == blockable) {
        LinkedList_RemoveEntry(blocked_list, tcb, list);
        LinkedList_AddEntry(ready_list, tcb, list);
        tcb->state = task_state::READY;
      }
    }

    g_mcu->TriggerPendSV();
  }
}

void Kernel::RegisterError(auto_task_stack_frame* args) {
  uint32_t pc = args->lr;
  auto next_task_stack_frame = args + 1;
  uint32_t sp = reinterpret_cast<uint32_t>(next_task_stack_frame);
  HandleErrorHook(pc, args->r0, args->r1, args->r2, args->r3, sp);
}

__WEAK void Kernel::TriggerSchedulerEntryHook() { }

__WEAK void Kernel::TriggerSchedulerExitHook() { }

__WEAK void Kernel::HandleErrorHook(uint32_t pc,
                                    uint32_t r0,
                                    uint32_t r1,
                                    uint32_t r2,
                                    uint32_t r3,
                                    uint32_t sp) { }

void Kernel::TriggerScheduler() {
  TriggerSchedulerEntryHook();
  uint64_t num_ticks = GetTicks();

  if (current_task &&
      current_task->state == task_state::RUNNING) {
    current_task->state = task_state::READY;
    current_task->run_last_timestamp = num_ticks;
  }

  // Select next task based on priority
  current_task = nullptr;
  task_control_block* tcb = nullptr;
  LinkedList_WalkEntry(ready_list, tcb, list) {
    if (tcb->state == task_state::READY) {
      if (!current_task || (tcb->priority > current_task->priority)) {
        current_task = tcb;
      } else if ((tcb->priority == current_task->priority) &&
                 (tcb->run_last_timestamp < current_task->run_last_timestamp)) {
        current_task = tcb;
      }
    }
  }

  if (current_task != nullptr) {
    current_task->state = task_state::RUNNING;
  }

  TriggerSchedulerExitHook();
}

void Kernel::TriggerScheduler_Static() {
  g_kernel->TriggerScheduler();
}

void Kernel::CheckTaskNeedsAwakening() {
  task_control_block* tcb = nullptr;
  task_control_block* next_tcb = nullptr;

  LinkedList_WalkEntry_Safe(sleeping_list, tcb, next_tcb, list) {
    // Resume task if asleep
    if (ticks >= tcb->blockArgument.timestamp) {
      tcb->state = task_state::READY;
      LinkedList_RemoveEntry(sleeping_list, tcb, list);
      LinkedList_AddEntry(ready_list, tcb, list);
    }
  }
}

void Kernel::HandleTick() {
  {
    CriticalSection s;
    ticks++;
  }
  CheckTaskNeedsAwakening();
  g_mcu->TriggerPendSV();
}

Kernel::Kernel() {
  g_kernel = this;
}

}  // namespace OS

__WEAK void App_SysTick_Hook() { }

CLINKAGE void SysTick_Handler() {
  App_SysTick_Hook();
  if (OS::g_kernel) {
    OS::g_kernel->HandleTick();
  }
}
