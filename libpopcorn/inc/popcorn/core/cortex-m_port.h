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

#ifndef POPCORN_CORE_CORTEX_M_PORT_H_
#define POPCORN_CORE_CORTEX_M_PORT_H_

#include <atomic>

#include "popcorn/platform.h"

#include "popcorn/core/syscall_idx.h"
#include "popcorn/core/kernel.h"

#ifndef UNITTEST
#define _SVC_CALL(x) asm volatile("svc %[opcode]" \
                     :: [opcode] "i" (static_cast<uint32_t>(x)));
#else
void _svc_call(Popcorn::SyscallIdx id);
#define _SVC_CALL(x) _svc_call(x)
#endif

#define EXC_RETURN_PSP_UNPRIV       (0xFFFFFFFDU)
#define XPSR_INIT_VALUE             (1U << 24)

CLINKAGE void SVC_Handler();

namespace Hw {

/**
 * @brief Automatically saved task stack frame upon exception entry.
 *        These are also the registers saved by the caller in the
 *        AAPCS (ARM Architecture Procedure Calling Standard).
 */
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

/**
 * @brief Manually saved task stack frame upon exception entry.
 *        These are the registers that are saved by the callee
 *        when required according to the AAPCS (ARM Architecture
 *        Procedure Calling Standard).
 */
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

/**
 * @brief Task stack frame used to switch tasks in the context change
 *        interrupt (PendSV). Contains both the registers automatically
 *        pushed to the stack by the processor, as well as the manually
 *        stored by us.
 */
struct task_stack_frame {
  manual_task_stack_frame manualsave;
  auto_task_stack_frame autosave;
};

class MCU {
 public:
  MCU();
  TEST_VIRTUAL void RegisterSyscallImpl(Popcorn::ISyscall* syscall_impl);
  TEST_VIRTUAL void Initialize() const;

  template<Popcorn::SyscallIdx id>
  static void SupervisorCall() {
    _SVC_CALL(id);
  }
  TEST_VIRTUAL void TriggerPendSV() const;
  TEST_VIRTUAL void DisableInterrupts();
  TEST_VIRTUAL void EnableInterrupts();

  TEST_VIRTUAL uint8_t* InitializeTask(uint8_t* stack_top,
                                       Popcorn::task_func func,
                                       void* arg) const;

  TEST_VIRTUAL ~MCU() = default;

 private:
  TEST_VIRTUAL void HandleSVC(auto_task_stack_frame* args) const;
  static void HandleSVC_Static(auto_task_stack_frame* args);
  Popcorn::SyscallIdx GetSVCCode(const std::uint8_t* pc) const;
  TEST_VIRTUAL void DisableInterruptsInternal() const;
  TEST_VIRTUAL void EnableInterruptsInternal() const;
  TEST_VIRTUAL task_stack_frame* AllocateTaskStackFrame(uint8_t* stack_ptr) const;

  Popcorn::ISyscall* m_syscall_impl;
  std::atomic_uint32_t m_nested_interrupt_level;

  friend void ::SVC_Handler();
  friend class MCUTest;
};
}  // namespace Hw

#endif  // POPCORN_CORE_CORTEX_M_PORT_H_
