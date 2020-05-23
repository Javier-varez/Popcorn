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

#ifndef TEST_INC_MOCKKERNEL_H_
#define TEST_INC_MOCKKERNEL_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Inc/kernel.h"

class MockKernel : public OS::Kernel {
 public:
  MockKernel() = default;
  MOCK_METHOD0(StartOS, void());
  MOCK_METHOD5(CreateTask, void(task_func func, void* arg, OS::Priority priority,
                                const char* name, std::uint32_t stack_size));
  MOCK_METHOD1(Sleep, void(std::uint32_t ticks));
  MOCK_METHOD0(DestroyTask, void());
  MOCK_METHOD0(Yield, void());
  MOCK_METHOD1(Wait, void(const OS::Blockable* Blockable));
  MOCK_METHOD1(RegisterError, void(OS::auto_task_stack_frame* args));
  MOCK_METHOD0(GetTicks, std::uint64_t());
  MOCK_METHOD0(TriggerScheduler, void());
  MOCK_METHOD0(HandleTick, void());
  MOCK_METHOD2(Lock, void(OS::Blockable* Blockable, bool available));
};

#endif  // TEST_INC_MOCKKERNEL_H_
