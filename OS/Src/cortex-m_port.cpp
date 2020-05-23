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

namespace Hw {
    MCU* g_mcu = nullptr;
    volatile SCB_t *g_SCB = reinterpret_cast<SCB_t*>(SCB_ADDR);

    MCU::MCU() {
        g_mcu = this;
    }

    void MCU::Initialize() {
        // Set OS IRQ priorities
        g_SCB->SHP[SYSTICK_SHP_IDX] = 0xFF;  // Minimum priority for SysTick
        g_SCB->SHP[PEND_SV_SHP_IDX] = 0xFF;  // Minimum priority for PendSV
        g_SCB->SHP[SVC_CALL_SHP_IDX] = 0x00;  // Maximum priority for SVC

        // Turn on stack alignment to 8 bytes on exception entry
        // On entry, the stacked value of the XPSR register will have
        // bit 9 set to 1 if the stack was aligned to 8 bytes.
        g_SCB->CCR |= SCB_CCR_STKALIGN;
    }

    void MCU::TriggerPendSV() {
        g_SCB->ICSR |= SCB_ICSR_PENDSVSET;
    }

    void MCU::SupervisorCall(OS::SyscallIdx svc) {
    }

    void MCU::HandleSVC_Static(struct OS::auto_task_stack_frame* args) {
        g_mcu->HandleSVC(args);
    }

    OS::SyscallIdx MCU::GetSVCCode(std::uint8_t* pc) {
        // First byte of svc instruction
        return static_cast<OS::SyscallIdx>(pc[-sizeof(std::uint16_t)]);
    }

    void MCU::HandleSVC(struct OS::auto_task_stack_frame* args) {
        std::uint8_t* pc = (std::uint8_t*)args->pc;
        OS::SyscallIdx svc_code = GetSVCCode(pc);
        // The arguments for the original function calls will be in
        // r0 to r3. If more arguments are required, they are placed in
        // the stack in order.

        // On exception entry, the stack must be 8 byte aligned
        // (if STKALIGN is 1) and it is possible that the processor
        // introduced padding in addition to just performing the stacking
        // of registers. This is why we need to check bit 9 of the stacked
        // XPSR register. This bit will be set to 1 if it was 4 byte aligned
        // on exception entry.
        uint32_t* original_call_stack = reinterpret_cast<uint32_t*>(args+1);
        bool fourByteAlignedOnEntry = (args->xpsr & (1 << 9)) != 0;
        if (fourByteAlignedOnEntry) {
            original_call_stack += 1;
        }
        switch (svc_code) {
        case OS::SyscallIdx::StartOS:
            OS::g_kernel->StartOS();
            break;
        case OS::SyscallIdx::CreateTask:
            OS::g_kernel->CreateTask(
                reinterpret_cast<task_func>(args->r1),
                static_cast<std::uintptr_t>(args->r2),
                static_cast<OS::Priority>(args->r3),
                reinterpret_cast<char*>(*original_call_stack),
                static_cast<std::uint32_t>(*(original_call_stack + 1)));
            break;
        case OS::SyscallIdx::Sleep:
            OS::g_kernel->Sleep(args->r1);
            break;
        case OS::SyscallIdx::DestroyTask:
            OS::g_kernel->DestroyTask();
            break;
        case OS::SyscallIdx::Yield:
            OS::g_kernel->Yield();
            break;
        case OS::SyscallIdx::Wait:
            OS::g_kernel->Wait(
                reinterpret_cast<const OS::Blockable*>(args->r1));
            break;
        case OS::SyscallIdx::Lock:
            OS::g_kernel->Lock(
                reinterpret_cast<OS::Blockable*>(args->r1),
                static_cast<bool>(args->r2));
            break;
        case OS::SyscallIdx::RegisterError:
        default:
            // TODO(javier_varez): Handle Register Error SVC call and register
            // backtrace
            OS::g_kernel->RegisterError(args);
            break;
        }
    }
}  // namespace Hw
