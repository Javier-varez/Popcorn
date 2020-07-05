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

#ifndef OS_INC_CORE_CORTEX_M_PORT_H_
#define OS_INC_CORE_CORTEX_M_PORT_H_

#include <atomic>

#include "Inc/platform.h"

#include "Inc/core/syscall_idx.h"
#include "Inc/core/kernel.h"

#ifndef UNITTEST
#define _SVC_CALL(x) asm volatile("svc %[opcode]" \
                     :: [opcode] "i" (static_cast<uint32_t>(x)));
#else
void _svc_call(OS::SyscallIdx id);
#define _SVC_CALL(x) _svc_call(x)
#endif

CLINKAGE void SVC_Handler();

namespace Hw {
class MCU {
 public:
  MCU();
  TEST_VIRTUAL void RegisterSyscallImpl(OS::ISyscall* syscall_impl);
  TEST_VIRTUAL void Initialize() const;

  template<OS::SyscallIdx id>
  static void SupervisorCall() {
    _SVC_CALL(id);
  }
  TEST_VIRTUAL void TriggerPendSV() const;
  TEST_VIRTUAL void DisableInterrupts();
  TEST_VIRTUAL void EnableInterrupts();

  TEST_VIRTUAL ~MCU() = default;

 private:
  TEST_VIRTUAL void HandleSVC(OS::auto_task_stack_frame* args) const;
  static void HandleSVC_Static(OS::auto_task_stack_frame* args);
  OS::SyscallIdx GetSVCCode(const std::uint8_t* pc) const;
  TEST_VIRTUAL void DisableInterruptsInternal() const;
  TEST_VIRTUAL void EnableInterruptsInternal() const;

  OS::ISyscall* m_syscall_impl;
  std::atomic_uint32_t m_nested_interrupt_level;

  friend void ::SVC_Handler();
  friend class MCUTest;
};
}  // namespace Hw

#endif  // OS_INC_CORE_CORTEX_M_PORT_H_
