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

#ifndef OS_INC_MUTEX_H_
#define OS_INC_MUTEX_H_

#include <cstdint>

#include "Inc/blockable.h"

namespace OS {
class Mutex: Blockable {
 public:
  Mutex();
  void Lock();
  void Unlock();

 private:
  std::uint8_t m_available;
  bool IsBlocked() const override;
};
}  // namespace OS

#endif  // OS_INC_MUTEX_H_
