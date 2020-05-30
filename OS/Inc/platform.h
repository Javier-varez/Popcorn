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

#ifndef OS_INC_PLATFORM_H_
#define OS_INC_PLATFORM_H_

#include <cstdint>

void AteAssert(const char* name, unsigned int line, bool condition);

#define CLINKAGE                    extern "C"
#define __STRINGIZE(_x)             #_x
#define __NAKED                     __attribute__((naked))
#define __WEAK                      __attribute__((weak))

#ifdef DISABLE_ASSERT
#define ATE_ASSERT(condition)
#else
#define ATE_ASSERT(condition)       AteAssert(__FILE__, __LINE__, condition)
#endif

#ifdef UNITTEST
#define TEST_VIRTUAL virtual
#else
#define TEST_VIRTUAL
#endif

#endif  // OS_INC_PLATFORM_H_
