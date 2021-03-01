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

#include "popcorn/core/cortex-m_port.h"
#include "popcorn/core/cortex-m_registers.h"
#include "popcorn/core/kernel.h"

#include "popcorn/API/syscall.h"

#include "popcorn/utils/memory_management.h"

#include "popcorn/os_config.h"


using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

using Popcorn::SyscallIdx;
using Popcorn::Priority;
using Popcorn::Lockable;

// Check the minimum task stack size is larger than the stack frame + alignment
static_assert(MINIMUM_TASK_STACK_SIZE >
              (sizeof(Hw::task_stack_frame) + sizeof(uint32_t)));

namespace Hw {

/**
 * @brief Global pointer to the singleton MCU instance.
 *        Used for static ISRs that need to call the MCU.
 */
MCU* g_mcu = nullptr;

/**
 * @brief SCB (System Control Block) registers pointer.
 */
volatile SCB_t *g_SCB = reinterpret_cast<SCB_t*>(SCB_ADDR);

/**
 * @brief SysTick (System Timer) registers pointer.
 */
volatile SysTick_t *g_SysTick = reinterpret_cast<SysTick_t*>(SYSTICK_ADDR);

MCU::MCU() :
  m_syscall_impl(nullptr),
  m_nested_interrupt_level(0) {
  g_mcu = this;
}

void MCU::RegisterSyscallImpl(Popcorn::ISyscall* syscall_impl) {
  m_syscall_impl = syscall_impl;
}

void MCU::Initialize() const {
  // Set OS IRQ priorities
  g_SCB->SHP[SYSTICK_SHP_IDX] = 0xFF;  // Minimum priority for SysTick
  g_SCB->SHP[PEND_SV_SHP_IDX] = 0xFF;  // Minimum priority for PendSV
  g_SCB->SHP[SVC_CALL_SHP_IDX] = 0x00;  // Maximum priority for SVC

  // Configure SysTick
  g_SysTick->LOAD = SYSTICK_SRC_CLK_FREQ_HZ / TICK_FREQ_HZ - 1;
  g_SysTick->VAL = 0U;
  g_SysTick->CTRL = SysTick_Ctrl_Enable
                  | SysTick_Ctrl_TickInt
                  | SysTick_Ctrl_ClkSource;

  // Turn on stack alignment to 8 bytes on exception entry
  // On entry, the stacked value of the XPSR register will have
  // bit 9 set to 1 if the stack was aligned to 8 bytes.
  g_SCB->CCR = SCB_CCR_STKALIGN | g_SCB->CCR;
}

void MCU::TriggerPendSV() const {
  g_SCB->ICSR = SCB_ICSR_PENDSVSET | g_SCB->ICSR;
}

void MCU::HandleSVC_Static(struct auto_task_stack_frame* args) {
  g_mcu->HandleSVC(args);
}

SyscallIdx MCU::GetSVCCode(const uint8_t* pc) const {
  // First byte of svc instruction
  return static_cast<SyscallIdx>(pc[-sizeof(uint16_t)]);
}

void MCU::HandleSVC(struct auto_task_stack_frame* args) const {
  const uint8_t* pc = reinterpret_cast<uint8_t*>(args->pc);
  SyscallIdx svc_code = GetSVCCode(pc);
  // The arguments for the original function calls will be in
  // r0 to r3. If more arguments are required, they are placed in
  // the stack in order. Look into the ARM EABI for details.
  // Another good document is the AAPCS, which documents the
  // ARM Architecture Procedure Calling Standard.

  // On exception entry, the stack must be 8 byte aligned
  // (if STKALIGN is 1) and it is possible that the processor
  // introduced padding in addition to just performing the stacking
  // of registers. This is why we need to check bit 9 of the stacked
  // XPSR register. This bit will be set to 1 if it was 4 byte aligned
  // on exception entry.
  uint32_t* original_call_stack = reinterpret_cast<uint32_t*>(args+1);
  bool fourByteAlignedOnEntry = args->xpsr & (1 << 9);
  if (fourByteAlignedOnEntry) {
    original_call_stack += 1;
  }
  ATE_ASSERT(m_syscall_impl != nullptr);

  switch (svc_code) {
    case SyscallIdx::StartOS: {
        m_syscall_impl->StartOS();
        break;
      }

    case SyscallIdx::CreateTask: {
        auto func = reinterpret_cast<Popcorn::task_func>(args->r1);
        auto arg = reinterpret_cast<void*>(args->r2);
        auto priority = static_cast<Priority>(args->r3);
        const auto* name = reinterpret_cast<char*>(*original_call_stack);
        auto stack_size = static_cast<uint32_t>(*(original_call_stack + 1));
        m_syscall_impl->CreateTask(func,
                             arg,
                             priority,
                             name,
                             stack_size);
        break;
      }

    case SyscallIdx::Sleep: {
        auto ticks = args->r1;
        m_syscall_impl->Sleep(ticks);
        break;
      }

    case SyscallIdx::DestroyTask: {
        m_syscall_impl->DestroyTask();
        break;
      }

    case SyscallIdx::Yield: {
        m_syscall_impl->Yield();
        break;
      }

    case SyscallIdx::Wait: {
        const auto* mutex = reinterpret_cast<Lockable*>(args->r1);
        ATE_ASSERT(mutex != nullptr);
        m_syscall_impl->Wait(*mutex);
        break;
      }

    case SyscallIdx::Lock: {
        auto* mutex = reinterpret_cast<Lockable*>(args->r1);
        auto acquired = static_cast<bool>(args->r2);
        ATE_ASSERT(mutex != nullptr);
        m_syscall_impl->Lock(*mutex, acquired);
        break;
      }

    case SyscallIdx::RegisterError:
    default: {
        /**
         * @todo (javier_varez) Handle Register Error SVC call and register
         *       backtrace
         */
        m_syscall_impl->RegisterError();
        break;
      }
  }
}

void MCU::DisableInterrupts() {
  auto level = m_nested_interrupt_level.fetch_add(1);
  if (level == 0) {
    DisableInterruptsInternal();
  }
}

void MCU::EnableInterrupts() {
  auto level = m_nested_interrupt_level.fetch_sub(1);
  ATE_ASSERT(level != 0);
  if (level == 1) {
    EnableInterruptsInternal();
  }
}

task_stack_frame* MCU::AllocateTaskStackFrame(uint8_t* stack_ptr) const {
  if (stack_ptr == nullptr) {
    return nullptr;
  }
  auto stack_frame = reinterpret_cast<uintptr_t>(stack_ptr);

  // Reserve enough space for the automatically stored task stack frame
  stack_frame -= sizeof(auto_task_stack_frame);

  // The automatically stored task frame needs to be aligned to 8 bytes
  // according to the AAPCS.
  stack_frame &= 0xFFFFFFF8;

  // Now reserve enough space for the manual task stack frame
  stack_frame -= sizeof(manual_task_stack_frame);

  return reinterpret_cast<task_stack_frame*>(stack_frame);
}

void DestroyTaskVeneer() {
  Popcorn::Syscall::Instance().DestroyTask();
}

uint8_t* MCU::InitializeTask(uint8_t* stack_top,
                             Popcorn::task_func func,
                             void* arg) const {
  if (nullptr == stack_top) {
    return nullptr;
  }

  auto* stack_frame_ptr = AllocateTaskStackFrame(stack_top);
  if (nullptr == stack_frame_ptr) {
    return nullptr;
  }

  auto lr = reinterpret_cast<uint32_t>(DestroyTaskVeneer);
  stack_frame_ptr->autosave.pc = reinterpret_cast<uint32_t>(func);
  stack_frame_ptr->autosave.r0 = reinterpret_cast<uintptr_t>(arg);
  stack_frame_ptr->autosave.xpsr = XPSR_INIT_VALUE;
  stack_frame_ptr->autosave.lr = lr;
  stack_frame_ptr->manualsave.lr = EXC_RETURN_PSP_UNPRIV;

  return reinterpret_cast<uint8_t*>(stack_frame_ptr);
}

/**
 * @brief Weak definition for testing purposes only.
 *        The actual implementation requires assembly
 *        and is located in cortex-m_port_asm.cpp
 */
__WEAK void MCU::DisableInterruptsInternal() const { }

/**
 * @brief Weak definition for testing purposes only.
 *        The actual implementation requires assembly
 *        and is located in cortex-m_port_asm.cpp
 */
__WEAK void MCU::EnableInterruptsInternal() const { }

/**
 * @brief Weak definition for testing purposes only.
 *        The actual implementation requires assembly
 *        and is located in cortex-m_port_asm.cpp
 * @return hardcoded to 0 in this fake implementation.
 */
__WEAK std::uintptr_t GetPC() {
  return 0;
}

}  // namespace Hw
