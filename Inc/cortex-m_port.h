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

#ifndef INC_CORTEX_M_PORT_H_
#define INC_CORTEX_M_PORT_H_

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

    template<OS::SyscallIdx svc_code>
    static void SupervisorCall() {
        #ifdef UNITTEST
        g_mcu->SupervisorCall(svc_code);
        #else
        asm volatile("svc %[opcode]"
            : : [opcode] "i" (static_cast<uint32_t>(svc_code)));
        #endif
    }
    TEST_VIRTUAL void TriggerPendSV();

    TEST_VIRTUAL ~MCU() = default;

 protected:
    TEST_VIRTUAL void SupervisorCall(OS::SyscallIdx svc);
    TEST_VIRTUAL void HandleSVC(struct OS::auto_task_stack_frame* args);
    static void HandleSVC_Static(struct OS::auto_task_stack_frame* args);
 private:
    OS::SyscallIdx GetSVCCode(std::uint8_t* pc);

    friend void ::SVC_Handler();
    friend class ::MCUTest;
};
}  // namespace Hw

#endif  // INC_CORTEX_M_PORT_H_
