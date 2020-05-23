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

extern "C" {
#include <stdint.h>
#include <cmsis_gcc.h>
}

#include "Inc/mutex.h"
#include "Inc/kernel.h"

namespace OS {
Mutex::Mutex() : available(true) { }

void Mutex::Lock() {
  bool done = false;
  do {
    if (__LDREXB(&available)) {
      done = __STREXB(0, &available) == 0;
    } else {
      // Sleep until the lock is free
      Block();
    }
  } while (!done);
  __CLREX();
  LockAcquired();
}

void Mutex::Unlock() {
  bool done = false;
  while (!done) {
    if (!__LDREXB(&available)) {
      done = __STREXB(1, &available) == 0;
    } else {
      Syscall::Instance().RegisterError();
    }
  }
  __CLREX();
  LockReleased();
}

bool Mutex::IsBlocked() const {
  // This should only be called within the OS.
  // No exclusive access required
  return !available;
}

}  // namespace OS
