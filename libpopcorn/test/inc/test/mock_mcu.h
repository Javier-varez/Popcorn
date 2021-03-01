/*
 * This file is part of the Popcorn
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

#ifndef TEST_INC_MOCKMCU_H_
#define TEST_INC_MOCKMCU_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "popcorn/core/cortex-m_port.h"

class MockMCU : public Hw::MCU {
 public:
  MockMCU() = default;
  MOCK_METHOD(void, RegisterSyscallImpl, (Popcorn::ISyscall*));
  MOCK_METHOD(void, Initialize, (), (const));
  MOCK_METHOD(void, TriggerPendSV, (), (const));
  MOCK_METHOD(uint8_t*, InitializeTask, (uint8_t* stack_top,
                                         Popcorn::task_func func,
                                         void* arg),
                                        (const));
};

class MCU_SVC {
 public:
  virtual void SupervisorCall(Popcorn::SyscallIdx) const = 0;
};

class MockSVC: public MCU_SVC {
 public:
  MOCK_METHOD(void, SupervisorCall, (Popcorn::SyscallIdx), (const));
};

namespace Hw {
  extern MCU* g_mcu;
  extern MockSVC* g_svc;
}  // namespace Hw

#endif  // TEST_INC_MOCKMCU_H_
