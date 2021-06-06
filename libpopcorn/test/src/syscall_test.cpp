/*
 * This file is part of Popcorn
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

#include <memory>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test/mock_mcu.h"

#include "popcorn/API/syscall.h"

#include "popcorn/core/syscall_idx.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::_;

using Hw::g_mcu;
using Popcorn::Syscall;
using Popcorn::SyscallIdx;
using Popcorn::Priority;
using Popcorn::Lockable;

using Hw::g_svc;

namespace Popcorn {

class SyscallTest: public ::testing::Test {
 private:
  void SetUp() override {
    syscall = &Syscall::Instance();
    g_mcu = &mcu;
    g_svc = &svc;
  }

  void TearDown() override { }

 protected:
  void SyscallWait(const Lockable& lockable) {
    syscall->Wait(lockable);
  }

  StrictMock<MockMCU> mcu;
  StrictMock<MockSVC> svc;
  Syscall* syscall;
};

TEST_F(SyscallTest, StartOS_Test) {
  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::StartOS));
  syscall->StartOS();
}

static void func(void*) {}
TEST_F(SyscallTest, CreateTask_Test) {
  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::CreateTask));
  syscall->CreateTask(func, 0, Priority::Level_0, "", 0);
}

TEST_F(SyscallTest, DestroyTask_Test) {
  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::DestroyTask));
  syscall->DestroyTask();
}

TEST_F(SyscallTest, Sleep_Test) {
  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Sleep));
  syscall->Sleep(1);
}

TEST_F(SyscallTest, Yield_Test) {
  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Yield));
  syscall->Yield();
}

TEST_F(SyscallTest, RegisterError_Test) {
  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::RegisterError));
  syscall->RegisterError();
}

TEST_F(SyscallTest, Wait_Test) {
  Lockable lockable;
  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Wait));
  SyscallWait(lockable);
}

}  // namespace Popcorn
