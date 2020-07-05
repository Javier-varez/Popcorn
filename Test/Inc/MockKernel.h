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
#include "Inc/core/kernel.h"

class MockKernel : public OS::Kernel {
 public:
  explicit MockKernel(Hw::MCU* mcu) : Kernel(mcu) { }
  MOCK_METHOD(void, StartOS, ());
  MOCK_METHOD(void, CreateTask, (OS::task_func func, void* arg,
                                 OS::Priority priority,
                                 const char* name, std::uint32_t stack_size));
  MOCK_METHOD(void, Sleep, (std::uint32_t ticks));
  MOCK_METHOD(void, DestroyTask, ());
  MOCK_METHOD(void, Yield, ());
  MOCK_METHOD(void, Wait, (const OS::Lockable&));
  MOCK_METHOD(void, RegisterError, ());
  MOCK_METHOD(std::uint64_t, GetTicks, ());
  MOCK_METHOD(void, TriggerScheduler, ());
  MOCK_METHOD(void, HandleTick, ());
  MOCK_METHOD(void, Lock, (OS::Lockable&, bool available));
};

#endif  // TEST_INC_MOCKKERNEL_H_
