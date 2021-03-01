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

#include <memory>
#include <atomic>
#include <thread>  // NOLINT
#include <chrono>  // NOLINT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test/mock_mcu.h"
#include "popcorn/primitives/spinlock.h"

using testing::StrictMock;
using testing::InSequence;
using testing::Invoke;

using std::unique_ptr;
using std::make_unique;
using std::atomic_flag;

class SpinLockTest: public ::testing::Test {
 private:
  void SetUp() override {
    Hw::g_mcu = &mcu;
    spinlock = make_unique<Popcorn::SpinLock>();
  }

 protected:
  StrictMock<MockMCU> mcu;
  unique_ptr<Popcorn::SpinLock> spinlock;

  atomic_flag& GetHeldFlag() {
    return spinlock->m_held;
  }
};

TEST_F(SpinLockTest, CheckInitialState) {
  EXPECT_EQ(GetHeldFlag().test_and_set(), false);
}

TEST_F(SpinLockTest, CheckCanBeAcquired) {
  spinlock->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);
}

TEST_F(SpinLockTest, CheckCanBeReleased) {
  spinlock->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);

  spinlock->Unlock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), false);
}

TEST_F(SpinLockTest, NotAcquiredUntilReleasedByOtherThread) {
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;
  using std::chrono::milliseconds;
  using std::this_thread::sleep_for;
  using std::thread;

  InSequence s;
  spinlock->Lock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), true);

  auto action = [this]() {
    using namespace std::literals::chrono_literals;  // NOLINT
    sleep_for(30ms);
    spinlock->Unlock();
  };

  thread t(action);
  auto first_point = high_resolution_clock::now();
  spinlock->Lock();
  auto second_point = high_resolution_clock::now();
  t.join();

  EXPECT_EQ(GetHeldFlag().test_and_set(), true);

  auto total_ms = duration_cast<milliseconds>(second_point - first_point).count();
  EXPECT_GE(total_ms, 30);

  spinlock->Unlock();
  EXPECT_EQ(GetHeldFlag().test_and_set(), false);
}
