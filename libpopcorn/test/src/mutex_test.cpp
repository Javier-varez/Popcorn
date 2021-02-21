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

#include "test/mock_mcu.h"
#include "popcorn/primitives/mutex.h"

using testing::StrictMock;
using testing::InSequence;
using testing::Invoke;

using std::unique_ptr;
using std::make_unique;
using std::atomic_flag;

using Hw::g_svc;

class MutexTest: public ::testing::Test {
 private:
  void SetUp() override {
    Hw::g_mcu = &mcu;
    g_svc = &svc;
    mutex = make_unique<Popcorn::Mutex>();
  }

  void TearDown() override {
    g_svc = nullptr;
  }

 protected:
  StrictMock<MockMCU> mcu;
  StrictMock<MockSVC> svc;
  unique_ptr<Popcorn::Mutex> mutex;

  atomic_flag& GetHeldFlag() {
    return mutex->m_held;
  }
};

TEST_F(MutexTest, CheckInitialState) {
  EXPECT_EQ(GetHeldFlag().test_and_set(), false);
}

TEST_F(MutexTest, CheckCanBeAcquired) {
  EXPECT_CALL(svc, SupervisorCall(Popcorn::SyscallIdx::Lock));
  mutex->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);
}

TEST_F(MutexTest, CheckCanBeReleased) {
  EXPECT_CALL(svc, SupervisorCall(Popcorn::SyscallIdx::Lock));
  mutex->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);

  EXPECT_CALL(svc, SupervisorCall(Popcorn::SyscallIdx::Lock));
  mutex->Unlock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), false);
}

TEST_F(MutexTest, NotAquiredUntilReleasedByOtherThread) {
  InSequence s;
  EXPECT_CALL(svc, SupervisorCall(Popcorn::SyscallIdx::Lock));
  mutex->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);

  auto release = [this](Popcorn::SyscallIdx idx) {
    EXPECT_CALL(svc, SupervisorCall(Popcorn::SyscallIdx::Lock));
    mutex->Unlock();
    EXPECT_EQ(GetHeldFlag().test_and_set(), false);
    GetHeldFlag().clear();
  };

  EXPECT_CALL(svc, SupervisorCall(Popcorn::SyscallIdx::Wait)).Times(10);
  EXPECT_CALL(svc, SupervisorCall(Popcorn::SyscallIdx::Wait)).WillOnce(Invoke(release));
  EXPECT_CALL(svc, SupervisorCall(Popcorn::SyscallIdx::Lock));
  mutex->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);

  EXPECT_CALL(svc, SupervisorCall(Popcorn::SyscallIdx::Lock));
  mutex->Unlock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), false);
}
