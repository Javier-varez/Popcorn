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

#include "Inc/API/syscall.h"

#include "Inc/core/syscall_idx.h"
#include "Inc/core/cortex-m_port.h"
#include "Inc/core/kernel.h"

using std::uint32_t;


namespace OS {
  Syscall& Syscall::Instance() {
    static Syscall syscall;
    static Hw::MCU mcu;
    static OS::Kernel kernel(&mcu);

    return syscall;
  }

  void Syscall::StartOS() {
    Hw::MCU::SupervisorCall<SyscallIdx::StartOS>();
    #ifndef UNITTEST
    while (1) {}  // Should never get here
    #endif
  }

  void Syscall::CreateTask(task_func func, void* arg, Priority priority,
                           const char* name, uint32_t stack_size) {
    Hw::MCU::SupervisorCall<SyscallIdx::CreateTask>();
  }

  void Syscall::DestroyTask() {
    Hw::MCU::SupervisorCall<SyscallIdx::DestroyTask>();
  }

  void Syscall::Sleep(uint32_t ticks) {
    Hw::MCU::SupervisorCall<SyscallIdx::Sleep>();
  }

  void Syscall::Yield() {
    Hw::MCU::SupervisorCall<SyscallIdx::Yield>();
  }

  void Syscall::Wait(const Lockable& lockable) {
    Hw::MCU::SupervisorCall<SyscallIdx::Wait>();
  }

  void Syscall::Lock(Lockable& lockable, bool acquired) {
    Hw::MCU::SupervisorCall<SyscallIdx::Lock>();
  }

  void Syscall::RegisterError() {
    Hw::MCU::SupervisorCall<SyscallIdx::RegisterError>();
  }

}  // namespace OS
