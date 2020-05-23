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

#include "Inc/kernel.h"
#include "Inc/cortex-m_port.h"

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
    [current_task_ptr] "r" (&OS::g_kernel->current_task),
    [exc_return] "i" (EXC_RETURN_PSP_UNPRIV),
    [TriggerScheduler] "r" (OS::Kernel::TriggerScheduler_Static)
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
  );
}
