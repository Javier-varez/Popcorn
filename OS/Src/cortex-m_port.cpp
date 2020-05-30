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

#include "Inc/cortex-m_port.h"
#include "Inc/cortex-m_registers.h"
#include "Inc/os_config.h"
#include "Inc/kernel.h"

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

using OS::SyscallIdx;
using OS::auto_task_stack_frame;
using OS::Priority;
using OS::Blockable;
using OS::g_kernel;

namespace Hw {

MCU* g_mcu = nullptr;
volatile SCB_t *g_SCB = reinterpret_cast<SCB_t*>(SCB_ADDR);
volatile SysTick_t *g_SysTick = reinterpret_cast<SysTick_t*>(SYSTICK_ADDR);

MCU::MCU() :
  nested_interrupt_level(0) {
  g_mcu = this;
}

void MCU::Initialize() {
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
  g_SCB->CCR |= SCB_CCR_STKALIGN;
}

void MCU::TriggerPendSV() {
  g_SCB->ICSR |= SCB_ICSR_PENDSVSET;
}

void MCU::HandleSVC_Static(struct auto_task_stack_frame* args) {
  g_mcu->HandleSVC(args);
}

SyscallIdx MCU::GetSVCCode(const uint8_t* pc) const {
  // First byte of svc instruction
  return static_cast<SyscallIdx>(pc[-sizeof(uint16_t)]);
}

void MCU::HandleSVC(struct auto_task_stack_frame* args) {
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

  switch (svc_code) {
    case SyscallIdx::StartOS: {
        g_kernel->StartOS();
        break;
      }

    case SyscallIdx::CreateTask: {
        auto func = reinterpret_cast<task_func>(args->r1);
        auto arg = reinterpret_cast<void*>(args->r2);
        auto priority = static_cast<Priority>(args->r3);
        const auto* name = reinterpret_cast<char*>(*original_call_stack);
        auto stack_size = static_cast<uint32_t>(*(original_call_stack + 1));
        g_kernel->CreateTask(func,
                                 arg,
                                 priority,
                                 name,
                                 stack_size);
        break;
      }

    case SyscallIdx::Sleep: {
        auto ticks = args->r1;
        g_kernel->Sleep(ticks);
        break;
      }

    case SyscallIdx::DestroyTask: {
        g_kernel->DestroyTask();
        break;
      }

    case SyscallIdx::Yield: {
        g_kernel->Yield();
        break;
      }

    case SyscallIdx::Wait: {
        const auto* mutex = reinterpret_cast<Blockable*>(args->r1);
        g_kernel->Wait(mutex);
        break;
      }

    case SyscallIdx::Lock: {
        auto* mutex = reinterpret_cast<Blockable*>(args->r1);
        auto acquired = static_cast<bool>(args->r2);
        g_kernel->Lock(mutex, acquired);
        break;
      }

    case SyscallIdx::RegisterError:
    default: {
        // TODO(javier_varez): Handle Register Error SVC call and register
        // backtrace
        g_kernel->RegisterError(args);
        break;
      }
  }
}

void MCU::DisableInterrupts() {
  auto level = nested_interrupt_level.fetch_add(1);
  if (level == 0) {
    DisableInterruptsInternal();
  }
}

void MCU::EnableInterrupts() {
  auto level = nested_interrupt_level.fetch_sub(1);
  ATE_ASSERT(level != 0);
  if (level == 1) {
    EnableInterruptsInternal();
  }
}

__WEAK void MCU::DisableInterruptsInternal() const { }
__WEAK void MCU::EnableInterruptsInternal() const { }

}  // namespace Hw
