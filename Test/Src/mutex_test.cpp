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
#include <atomic>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Test/Inc/MockMCU.h"
#include "Inc/mutex.h"

using testing::StrictMock;
using testing::InSequence;
using testing::Invoke;

using std::unique_ptr;
using std::make_unique;
using std::atomic_flag;

class MutexTest: public ::testing::Test {
 private:
  void SetUp() override {
    Hw::g_mcu = &mcu;
    mutex = make_unique<OS::Mutex>();
  }

 protected:
  StrictMock<MockMCU> mcu;
  unique_ptr<OS::Mutex> mutex;

  atomic_flag& GetHeldFlag() {
    return mutex->m_held;
  }
};

TEST_F(MutexTest, CheckInitialState) {
  EXPECT_EQ(GetHeldFlag().test_and_set(), false);
}

TEST_F(MutexTest, CheckCanBeAcquired) {
  EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Lock));
  mutex->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);
}

TEST_F(MutexTest, CheckCanBeReleased) {
  EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Lock));
  mutex->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);

  EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Lock));
  mutex->Unlock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), false);
}

TEST_F(MutexTest, NotAquiredUntilReleasedByOtherThread) {
  InSequence s;
  EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Lock));
  mutex->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);

  auto release = [this](OS::SyscallIdx idx) {
    EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Lock));
    mutex->Unlock();
    EXPECT_EQ(GetHeldFlag().test_and_set(), false);
    GetHeldFlag().clear();
  };

  EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Wait)).Times(10);
  EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Wait)).WillOnce(Invoke(release));
  EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Lock));
  mutex->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);

  EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Lock));
  mutex->Unlock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), false);
}
