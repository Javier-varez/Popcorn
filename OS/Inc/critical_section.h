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

#ifndef OS_INC_CRITICAL_SECTION_H_
#define OS_INC_CRITICAL_SECTION_H_

#include "Inc/cortex-m_port.h"

namespace OS {
class CriticalSection {
 public:
  inline CriticalSection() {
    Hw::g_mcu->DisableInterrupts();
  }

  inline ~CriticalSection() {
    Hw::g_mcu->EnableInterrupts();
  }
};
}  // namespace OS

#endif  // OS_INC_CRITICAL_SECTION_H_
