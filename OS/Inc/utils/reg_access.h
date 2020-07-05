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

#ifndef OS_INC_UTILS_REG_ACCESS_H_
#define OS_INC_UTILS_REG_ACCESS_H_

#include <cstdint>

namespace Hw {

class RegAccess {
 public:
  using Register = volatile std::uint32_t*;

  static void Write(Register addr, std::uint32_t value) {
    *addr = value;
  }

  static std::uint32_t Read(Register addr) {
    return *addr;
  }

  static void Modify(Register addr, std::uint32_t mask, std::uint32_t value) {
    std::uint32_t temp = *addr;
    *addr = (temp & mask) | value;
  }
};

}  // namespace Hw

#endif  // OS_INC_UTILS_REG_ACCESS_H_
