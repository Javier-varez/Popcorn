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

#ifndef OS_INC_CORTEX_M_PORT_H_
#define OS_INC_CORTEX_M_PORT_H_

#include <atomic>

#include "Inc/platform.h"
#include "Inc/syscall_idx.h"
#include "Inc/kernel.h"

CLINKAGE void SVC_Handler();

class MCUTest;

namespace Hw {
class MCU;
extern MCU* g_mcu;

class MCU {
 public:
  MCU();
  TEST_VIRTUAL void Initialize();

  template<OS::SyscallIdx id>
  static void SupervisorCall() {
    #ifdef UNITTEST
    g_mcu->SupervisorCall(id);
    #else
    asm volatile("svc %[opcode]"
        : : [opcode] "i" (static_cast<uint32_t>(id)));
    #endif
  }
  TEST_VIRTUAL void TriggerPendSV();
  TEST_VIRTUAL void DisableInterrupts();
  TEST_VIRTUAL void EnableInterrupts();

  TEST_VIRTUAL ~MCU() = default;

 protected:
  TEST_VIRTUAL void SupervisorCall(OS::SyscallIdx svc);
  TEST_VIRTUAL void HandleSVC(OS::auto_task_stack_frame* args);
  static void HandleSVC_Static(OS::auto_task_stack_frame* args);

 private:
  std::atomic_uint32_t nested_interrupt_level;

  OS::SyscallIdx GetSVCCode(const std::uint8_t* pc) const;
  TEST_VIRTUAL void DisableInterruptsInternal() const;
  TEST_VIRTUAL void EnableInterruptsInternal() const;

  friend void ::SVC_Handler();
  friend class ::MCUTest;
};
}  // namespace Hw

#endif  // OS_INC_CORTEX_M_PORT_H_
