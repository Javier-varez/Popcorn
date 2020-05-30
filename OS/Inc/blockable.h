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

#ifndef OS_INC_BLOCKABLE_H_
#define OS_INC_BLOCKABLE_H_

#include "Inc/syscall.h"

namespace OS {
// Mutex, lists, etc, must be sublcass of Blockable
class Blockable {
 protected:
  void Block() {
    Syscall::Instance().Wait(*this);
  }
  void LockAcquired() {
    Syscall::Instance().Lock(*this, true);
  }
  void LockReleased() {
    Syscall::Instance().Lock(*this, false);
  }
  struct task_control_block *m_blocker;
  friend class Kernel;
};
}  // namespace OS

#endif  // OS_INC_BLOCKABLE_H_
