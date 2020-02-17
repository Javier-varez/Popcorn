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

#include <memory>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Test/Inc/MockMCU.h"
#include "Inc/syscall.h"
#include "Inc/syscall_idx.h"


using ::testing::StrictMock;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::_;

class SyscallTest: public ::testing::Test {
 private:
    void SetUp() override {
        syscall = &OS::Syscall::Instance();
        Hw::g_mcu = &mcu;
    }

    void TearDown() override { }

 protected:
    void SyscallWait(const OS::Blockable& blockable) {
        syscall->Wait(blockable);
    }

    StrictMock<Hw::MockMCU> mcu;
    OS::Syscall* syscall;
};

TEST_F(SyscallTest, StartOS_Test) {
    EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::StartOS));
    syscall->StartOS();
}

static void func(void*) {}
TEST_F(SyscallTest, CreateTask_Test) {
    EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::CreateTask));
    syscall->CreateTask(func, 0, OS::Priority::Level_0, "", 0);
}

TEST_F(SyscallTest, DestroyTask_Test) {
    EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::DestroyTask));
    syscall->DestroyTask();
}

TEST_F(SyscallTest, Sleep_Test) {
    EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Sleep));
    syscall->Sleep(1);
}

TEST_F(SyscallTest, Yield_Test) {
    EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Yield));
    syscall->Yield();
}

TEST_F(SyscallTest, RegisterError_Test) {
    EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::RegisterError));
    syscall->RegisterError();
}

TEST_F(SyscallTest, Wait_Test) {
    OS::Blockable blockable;
    EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Wait));
    SyscallWait(blockable);
}
