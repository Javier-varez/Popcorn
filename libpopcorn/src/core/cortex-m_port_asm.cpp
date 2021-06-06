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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "popcorn/core/cortex-m_port.h"
#include "popcorn/core/kernel.h"

namespace Popcorn {
  extern Kernel *g_kernel;
}  // namespace Popcorn

namespace Hw {
  // Use this ASM symbol to force the linker to load symbols from this translation unit.
  // Otherwise, since all of these symbols are already defined outside (even if weak)
  // they are ignored.
  volatile uint32_t dummy_asm_symbol;
}  // namespace HW

CLINKAGE __NAKED void PendSV_Handler() {
  asm volatile (
    "               ldr r1, [%[current_task_ptr]]     \n"
    "               cbz r1, TaskSwitch                \n"
    "               mrs r0, psp                       \n"
    "               stmdb r0!, {r4-r11, r14}          \n"
    "               str r0, [r1]                      \n" // scheduler.current_task->stack_ptr = psp
    "TaskSwitch:    push {%[current_task_ptr], lr}    \n"
    "               blx %[TriggerScheduler]           \n"
    "               pop {%[current_task_ptr], lr}     \n"
    "               ldr r1, [%[current_task_ptr]]     \n"
    "               cbz r1, RetISR                    \n"
    "               ldr r0, [r1]                      \n"
    "               ldmia r0!, {r4-r11, r14}          \n"
    "               msr psp, r0                       \n" // psp = scheduler.current_task->stack_ptr
    "               isb                               \n"
    "               mov lr, %[exc_return]             \n"
    "RetISR:        bx lr                             \n"
    : :
    [current_task_ptr] "r" (&Popcorn::g_kernel->m_current_task),
    [exc_return] "i" (EXC_RETURN_PSP_UNPRIV),
    [TriggerScheduler] "r" (Popcorn::Kernel::TriggerScheduler_Static)
    : "r1", "r0", "lr"
  );
}

CLINKAGE __NAKED void SVC_Handler() {
  asm volatile (
    "    tst lr, #4              \n"
    "    ite eq                  \n"
    "    mrseq r0, msp           \n"
    "    mrsne r0, psp           \n"
    "    bx %[svc_handler]       \n"
    : : [svc_handler] "r" (Hw::MCU::HandleSVC_Static)
    : "r0", "lr"
  );
}

namespace Hw {
void MCU::DisableInterruptsInternal() const {
  asm volatile("cpsid i");
}

void MCU::EnableInterruptsInternal() const {
  asm volatile("cpsie i");
}

std::uintptr_t GetPC() {
  std::uintptr_t lr = 0;
  asm volatile (
    "mov %[link_reg], lr \n"
  : : [link_reg] "r" (lr));
  return lr;
}

}  // namespace Hw
