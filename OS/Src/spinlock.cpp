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

extern "C" {
#include <stdint.h>
#include <cmsis_gcc.h>
}

#include "Inc/spinlock.h"
#include "Inc/syscall.h"


namespace OS {
    SpinLock::SpinLock() :
        available(true) { }

    void SpinLock::Lock() {
        bool done = false;
        while (!done) {
            if (__LDREXB(&available)) {
                done = __STREXB(false, &available) == 0;
            }
        }
        __CLREX();
    }

    void SpinLock::Unlock() {
        bool done = false;
        while (!done) {
            if (!__LDREXB(&available)) {
                done = __STREXB(true, &available) == 0;
            } else {
                OS::Syscall::Instance().RegisterError();
            }
        }
        __CLREX();
    }
}  // namespace OS