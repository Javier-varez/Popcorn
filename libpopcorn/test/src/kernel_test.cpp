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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test/mock_mcu.h"
#include "test/mock_mem_management.h"
#include "popcorn/core/kernel.h"
#include "popcorn/core/syscall_idx.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::AnyNumber;
using ::testing::_;

using std::uint8_t;
using std::uint32_t;
using std::uint64_t;
using std::uintptr_t;
using std::make_unique;
using std::unique_ptr;

using Popcorn::task_control_block;
using Popcorn::Kernel;
using Popcorn::Priority;
using Popcorn::task_state;
using Popcorn::Lockable;
using Popcorn::SyscallIdx;

using Hw::task_stack_frame;

using Hw::g_svc;

namespace Popcorn {
void IdleTask(void *arg);
}  // namespace Popcorn

class KernelTest: public ::testing::Test {
 public:
  constexpr static uint32_t kStackSize = 1024;

 private:
  void SetUp() override {
    g_MockMemManagement = &memManagement;
    g_svc = &svc;
    EXPECT_CALL(mcu, RegisterSyscallImpl(_));
    kernel = make_unique<Kernel>(&mcu);

    memset(idleStack, 0xA5, sizeof(idleStack));
    memset(task1Stack, 0xA5, sizeof(task1Stack));
    memset(task2Stack, 0xA5, sizeof(task2Stack));
    memset(task3Stack, 0xA5, sizeof(task3Stack));
    memset(&idleTCB, 0xA5, sizeof(idleTCB));
    memset(&task1TCB, 0xA5, sizeof(task1TCB));
    memset(&task2TCB, 0xA5, sizeof(task2TCB));
    memset(&task3TCB, 0xA5, sizeof(task3TCB));
  }

  void TearDown() override {
    g_MockMemManagement = nullptr;
    g_svc = nullptr;
  }

 protected:
  LinkedList_t* GetReadyTaskList() {
    return kernel->m_ready_list;
  }

  task_control_block* GetCurrentTask() {
    return kernel->m_current_task;
  }

  uint64_t GetTicks() {
    return kernel->m_ticks;
  }

  void SetTicks(uint64_t ticks) {
    kernel->m_ticks = ticks;
  }

  void HandleTick() {
    kernel->HandleTick();
  }

  void TriggerScheduler() {
    kernel->TriggerScheduler();
  }

  void SetCurrentTask(task_control_block* tcb) {
    kernel->m_current_task = tcb;
  }

  static void TaskFunction(void* arg) { }

  void CreateTask(Priority priority,
                  task_control_block* tcb,
                  uint8_t* stack) {
    EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
        .Times(1).WillOnce(Return(tcb)).RetiresOnSaturation();
    EXPECT_CALL(memManagement, Malloc(kStackSize))
        .Times(1).WillOnce(Return(stack)).RetiresOnSaturation();
    EXPECT_CALL(mcu, InitializeTask(stack + kStackSize,
                                    TaskFunction,
                                    nullptr))
                                    .WillOnce(Return(stack + kStackSize - 10));
    kernel->CreateTask(TaskFunction, nullptr, priority, "TaskName", kStackSize);
  }

  void StartOS() {
    EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
        .Times(1).WillOnce(Return(&idleTCB)).RetiresOnSaturation();
    EXPECT_CALL(memManagement, Malloc(MINIMUM_TASK_STACK_SIZE))
        .Times(1).WillOnce(Return(idleStack)).RetiresOnSaturation();
    EXPECT_CALL(mcu, InitializeTask(idleStack + MINIMUM_TASK_STACK_SIZE,
        Popcorn::IdleTask, nullptr))
        .WillOnce(Return(idleStack + MINIMUM_TASK_STACK_SIZE - 10));
    EXPECT_CALL(mcu, Initialize()).Times(1).RetiresOnSaturation();
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
    kernel->StartOS();
  }

  StrictMock<MockMCU> mcu;
  StrictMock<MockMemManagement> memManagement;
  StrictMock<MockSVC> svc;
  unique_ptr<Kernel> kernel;
  static uint8_t idleStack[MINIMUM_TASK_STACK_SIZE];
  static uint8_t task1Stack[kStackSize];
  static uint8_t task2Stack[kStackSize];
  static uint8_t task3Stack[kStackSize];
  task_control_block idleTCB;
  task_control_block task1TCB;
  task_control_block task2TCB;
  task_control_block task3TCB;
};

uint8_t KernelTest::idleStack[] __attribute__((aligned(8)));
uint8_t KernelTest::task1Stack[] __attribute__((aligned(8)));
uint8_t KernelTest::task2Stack[] __attribute__((aligned(8)));
uint8_t KernelTest::task3Stack[] __attribute__((aligned(8)));


TEST_F(KernelTest, StackPtrIsFirstFieldInTCB) {
  // Assembly code relies on the stack_ptr being at
  // offset 0 from the task_control block
  task_control_block tcb;
  ASSERT_EQ(reinterpret_cast<void*>(&tcb),
            reinterpret_cast<void*>(&tcb.stack_ptr));
}

TEST_F(KernelTest, CreateTaskInitializesTCB_Test) {
  uint8_t arg = 0;
  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task1Stack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task1TCB)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(task1Stack + kStackSize,
                                  TaskFunction,
                                  &arg))
                                  .WillOnce(Return(task1Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction,
                     &arg,
                     Priority::Level_7,
                     "NewTask",
                     kStackSize);

  ASSERT_EQ(CONTAINER_OF(GetReadyTaskList(), task_control_block, list),
    &task1TCB);
  ASSERT_STREQ(task1TCB.name, "NewTask");
  ASSERT_EQ(task1TCB.state, task_state::READY);
  ASSERT_EQ(task1TCB.priority, Priority::Level_7);
  ASSERT_EQ(task1TCB.arg, reinterpret_cast<uintptr_t>(&arg));
  ASSERT_EQ(task1TCB.func, &TaskFunction);
  ASSERT_EQ(task1TCB.stack_base, (uintptr_t)task1Stack);
  ASSERT_EQ(task1TCB.run_last_timestamp, 0ULL);
  ASSERT_EQ(reinterpret_cast<uint8_t*>(task1TCB.stack_ptr),
    task1Stack + kStackSize - 10);
}

TEST_F(KernelTest, StartOS_Test) {
  EXPECT_CALL(mcu, Initialize()).Times(1);
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1);

  // Idle allocation
  EXPECT_CALL(memManagement, Malloc(MINIMUM_TASK_STACK_SIZE))
    .Times(1).WillOnce(Return(idleStack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&idleTCB)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(idleStack + MINIMUM_TASK_STACK_SIZE,
    Popcorn::IdleTask, nullptr))
    .WillOnce(Return(idleStack + MINIMUM_TASK_STACK_SIZE - 10));
  kernel->StartOS();

  ASSERT_EQ(CONTAINER_OF(GetReadyTaskList(), task_control_block, list),
    &idleTCB);
  ASSERT_STREQ(idleTCB.name, "Idle");
  ASSERT_EQ(idleTCB.state, task_state::READY);
  ASSERT_EQ(idleTCB.priority, Priority::IDLE);
  ASSERT_EQ(idleTCB.arg, 0U);
  ASSERT_EQ(idleTCB.func, &Popcorn::IdleTask);
  ASSERT_EQ(idleTCB.stack_base, (uintptr_t)idleStack);
  ASSERT_EQ(reinterpret_cast<uint8_t*>(idleTCB.stack_ptr),
            idleStack + MINIMUM_TASK_STACK_SIZE - 10);
}

TEST_F(KernelTest, Yield_Test) {
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1);
  kernel->Yield();
}

TEST_F(KernelTest, CreateTask_Test) {
  uint32_t args[] = { 128, 125 };

  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task1Stack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task1TCB)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(task1Stack + kStackSize,
                                  TaskFunction,
                                  &args[0]))
                                  .WillOnce(Return(task1Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &args[0], Priority::Level_3,
                     "TestTask1", kStackSize);

  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task2Stack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task2TCB)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(task2Stack + kStackSize,
                                  TaskFunction,
                                  &args[1]))
                                  .WillOnce(Return(task2Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &args[1], Priority::Level_7,
                       "TestTask2", kStackSize);

  int i = 0;
  task_control_block* tcb = nullptr;
  LinkedList_WalkEntry(GetReadyTaskList(), tcb, list) {
    task_control_block* expected_tcb = nullptr;
    switch (i) {
      case 0:
        expected_tcb = &task1TCB;
        break;
      case 1:
        expected_tcb = &task2TCB;
        break;
      default:
        FAIL();
        break;
    }

    ASSERT_EQ(tcb, expected_tcb);
    i++;
  }
}

TEST_F(KernelTest, CreateTask_MallocFail_Test) {
  uint8_t arg = 0;
  EXPECT_CALL(memManagement, Malloc(_))
    .Times(AnyNumber()).WillRepeatedly(Return(nullptr));
  kernel->CreateTask(TaskFunction, &arg, Priority::Level_3,
                     "Nulltask", MINIMUM_TASK_STACK_SIZE);

  EXPECT_EQ(GetReadyTaskList(), nullptr);
}

TEST_F(KernelTest, DestroyTask_Test) {
  uint32_t args[] = { 128, 125 };

  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task1Stack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task1TCB)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(task1Stack + kStackSize,
                                  TaskFunction,
                                  &args[0]))
                                  .WillOnce(Return(task1Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &args[0], Priority::Level_3,
                     "TestTask1", kStackSize);

  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task2Stack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task2TCB)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(task2Stack + kStackSize,
    TaskFunction, &args[1]))
    .WillOnce(Return(task2Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &args[1], Priority::Level_7,
                     "TestTask2", kStackSize);

  // Set first task as current task
  SetCurrentTask(CONTAINER_OF(GetReadyTaskList(), task_control_block, list));

  EXPECT_CALL(memManagement, Free(task1Stack))
    .Times(1).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Free(&task1TCB))
    .Times(1).RetiresOnSaturation();
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1);
  kernel->DestroyTask();

  int i = 0;
  task_control_block* tcb = nullptr;
  LinkedList_WalkEntry(GetReadyTaskList(), tcb, list) {
    task_control_block *expected_tcb = nullptr;
    switch (i) {
    case 0:
      expected_tcb = &task2TCB;
      break;

    default:
      FAIL();
      break;
    }
    EXPECT_EQ(tcb, expected_tcb);
    i++;
  }
}

TEST_F(KernelTest, Ticks_Test) {
  EXPECT_EQ(kernel->GetTicks(), 0ULL);
  EXPECT_EQ(GetTicks(), 0ULL);

  for (uint64_t i = 0; i < 200; i++) {
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
    HandleTick();

    EXPECT_EQ(kernel->GetTicks(), i+1);
    EXPECT_EQ(GetTicks(), i+1);
  }
}

TEST_F(KernelTest, TriggerScheduler_Test) {
  uint8_t arg = 0;
  EXPECT_CALL(memManagement, Malloc(_))
    .Times(1).WillOnce(Return(task1Stack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task1TCB)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(task1Stack + kStackSize,
    TaskFunction, &arg)).WillOnce(Return(task1Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &arg, Priority::Level_3,
                     "TestTask1", kStackSize);

  EXPECT_CALL(mcu, Initialize()).Times(1);
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1);
  EXPECT_CALL(memManagement, Malloc(MINIMUM_TASK_STACK_SIZE))
    .Times(1).WillOnce(Return(idleStack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&idleTCB)).RetiresOnSaturation();

  EXPECT_CALL(mcu, InitializeTask(idleStack + MINIMUM_TASK_STACK_SIZE,
    Popcorn::IdleTask, nullptr))
    .WillOnce(Return(task1Stack + MINIMUM_TASK_STACK_SIZE - 10));
  kernel->StartOS();

  EXPECT_EQ(GetCurrentTask(), nullptr);

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task1TCB);
}

TEST_F(KernelTest, Sleep_Test) {
  constexpr uint64_t SLEEP_TICKS = 1242;
  SetTicks(0xFFFFFFFFULL);

  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task1Stack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task1TCB)).RetiresOnSaturation();
  uint8_t arg = 0;
  EXPECT_CALL(mcu, InitializeTask(task1Stack + kStackSize,
    TaskFunction, &arg))
    .WillOnce(Return(task1Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &arg, Priority::Level_3,
                     "TestTask1", kStackSize);

  EXPECT_CALL(mcu, Initialize()).Times(1);
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1);
  EXPECT_CALL(memManagement, Malloc(MINIMUM_TASK_STACK_SIZE))
    .Times(1).WillOnce(Return(idleStack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&idleTCB)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(idleStack + MINIMUM_TASK_STACK_SIZE,
    Popcorn::IdleTask, nullptr))
    .WillOnce(Return(task1Stack + MINIMUM_TASK_STACK_SIZE - 10));
  kernel->StartOS();

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task1TCB);

  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  kernel->Sleep(SLEEP_TICKS);

  TriggerScheduler();

  for (uint32_t i = 0; i < SLEEP_TICKS; i++) {
    EXPECT_EQ(GetCurrentTask(), &idleTCB);
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
    HandleTick();
    TriggerScheduler();
  }

  EXPECT_EQ(GetCurrentTask(), &task1TCB);
}

namespace {
class FakeBlock: public Lockable {
 public:
  explicit FakeBlock(Kernel* kernel) :
    m_kernel(kernel) { }

  void Lock() {
    if (m_blocked) {
      Block();
      m_kernel->Wait(*this);
    } else {
      m_blocked = true;
      LockAcquired();
      m_kernel->Lock(*this, true);
    }
  }

  void Unlock() {
    m_blocked = false;
    LockReleased();
    m_kernel->Lock(*this, false);
  }

 private:
  Kernel* m_kernel;
  bool m_blocked = false;
};

}  // namespace

TEST_F(KernelTest, Wait_Test) {
  FakeBlock block(kernel.get());

  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task1Stack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task1TCB)).RetiresOnSaturation();
  uint8_t arg = 0;
  EXPECT_CALL(mcu, InitializeTask(task1Stack + kStackSize,
    TaskFunction, &arg))
    .WillOnce(Return(task1Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &arg, Priority::Level_2,
                     "TestTask1", kStackSize);

  EXPECT_CALL(mcu, Initialize()).Times(1);
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1);
  EXPECT_CALL(memManagement, Malloc(MINIMUM_TASK_STACK_SIZE))
    .Times(1).WillOnce(Return(idleStack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&idleTCB)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(idleStack + MINIMUM_TASK_STACK_SIZE,
    Popcorn::IdleTask, nullptr))
    .WillOnce(Return(task1Stack + MINIMUM_TASK_STACK_SIZE - 10));
  kernel->StartOS();

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task1TCB);

  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task2Stack)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task2TCB)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(task2Stack + kStackSize,
    TaskFunction, &arg))
    .WillOnce(Return(task2Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &arg, Priority::Level_3,
                     "TestTask2", kStackSize);

  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Lock));
  block.Lock();

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task2TCB);

  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Wait));
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  block.Lock();

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task1TCB);

  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Lock));
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  block.Unlock();

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task2TCB);
}

TEST_F(KernelTest, Priority_Test) {
  FakeBlock block(kernel.get());

  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task1TCB)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task1Stack)).RetiresOnSaturation();
  uint8_t arg = 0;
  EXPECT_CALL(mcu, InitializeTask(task1Stack + kStackSize,
    TaskFunction, &arg))
    .WillOnce(Return(task1Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &arg, Priority::Level_1,
                     "TestTask1", kStackSize);

  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&idleTCB)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(MINIMUM_TASK_STACK_SIZE))
    .Times(1).WillOnce(Return(idleStack)).RetiresOnSaturation();
  EXPECT_CALL(mcu, Initialize()).Times(1).RetiresOnSaturation();
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(idleStack + MINIMUM_TASK_STACK_SIZE,
    Popcorn::IdleTask, nullptr))
    .WillOnce(Return(task1Stack + MINIMUM_TASK_STACK_SIZE - 10));
  kernel->StartOS();

  TriggerScheduler();
  EXPECT_NE(GetCurrentTask(), nullptr);
  EXPECT_STREQ(GetCurrentTask()->name, "TestTask1");

  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Lock));
  block.Lock();

  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task2TCB)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task2Stack)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(task2Stack + kStackSize,
    TaskFunction, &arg))
    .WillOnce(Return(task2Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &arg, Priority::Level_3,
                     "TestTask2", kStackSize);

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task2TCB);

  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Wait));
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  block.Lock();

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task1TCB);

  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task3TCB)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task3Stack)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(task3Stack + kStackSize,
    TaskFunction, &arg))
    .WillOnce(Return(task3Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &arg, Priority::Level_2,
                     "TestTask3", kStackSize);

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task1TCB);

  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Lock));
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  block.Unlock();

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task2TCB);
}

TEST_F(KernelTest, EqualPriorityAppliesRoundRobin_Test) {
  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task1TCB)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task1Stack)).RetiresOnSaturation();
  uint8_t arg = 0;
  EXPECT_CALL(mcu, InitializeTask(task1Stack + kStackSize,
    TaskFunction, &arg))
    .WillOnce(Return(task1Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &arg, Priority::Level_1,
                     "TestTask1", kStackSize);

  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&idleTCB)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(MINIMUM_TASK_STACK_SIZE))
    .Times(1).WillOnce(Return(idleStack)).RetiresOnSaturation();
  EXPECT_CALL(mcu, Initialize()).Times(1).RetiresOnSaturation();
  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(idleStack + MINIMUM_TASK_STACK_SIZE,
    Popcorn::IdleTask, nullptr))
    .WillOnce(Return(task1Stack + MINIMUM_TASK_STACK_SIZE - 10));
  kernel->StartOS();

  EXPECT_CALL(memManagement, Malloc(sizeof(task_control_block)))
    .Times(1).WillOnce(Return(&task2TCB)).RetiresOnSaturation();
  EXPECT_CALL(memManagement, Malloc(kStackSize))
    .Times(1).WillOnce(Return(task2Stack)).RetiresOnSaturation();
  EXPECT_CALL(mcu, InitializeTask(task2Stack + kStackSize,
    TaskFunction, &arg))
    .WillOnce(Return(task2Stack + kStackSize - 10));
  kernel->CreateTask(TaskFunction, &arg, Priority::Level_1,
                     "TestTask2", kStackSize);

  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  HandleTick();
  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task1TCB);

  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  HandleTick();
  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task2TCB);

  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  HandleTick();
  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task1TCB);

  EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
  HandleTick();
  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task2TCB);
}

TEST_F(KernelTest, PriorityInheritanceAvoidsPriorityInversion_Test) {
  FakeBlock mutex(kernel.get());

  CreateTask(Priority::Level_0, &task1TCB, task1Stack);

  StartOS();

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task1TCB);

  // Task 1 takes the mutex
  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Lock));
  mutex.Lock();

  CreateTask(Priority::Level_1, &task2TCB, task2Stack);

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task2TCB);

  CreateTask(Priority::Level_2, &task3TCB, task3Stack);

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task3TCB);

  // Task 3 attempts to take the mutex
  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Wait));
  EXPECT_CALL(mcu, TriggerPendSV());
  mutex.Lock();

  // Now task 1 should inherit the priory of task 3 and
  // execute instead of task 2.
  EXPECT_EQ(task1TCB.priority, task3TCB.priority);
  EXPECT_EQ(task1TCB.base_priority, Priority::Level_0);
  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task1TCB);

  // Task 1 unlocks the mutex and task 3 resumes execution
  EXPECT_CALL(svc, SupervisorCall(SyscallIdx::Lock));
  EXPECT_CALL(mcu, TriggerPendSV());
  mutex.Unlock();

  // Priority must be restored
  EXPECT_EQ(task1TCB.priority, task1TCB.base_priority);
  EXPECT_EQ(task1TCB.base_priority, Priority::Level_0);

  TriggerScheduler();
  EXPECT_EQ(GetCurrentTask(), &task3TCB);
}
