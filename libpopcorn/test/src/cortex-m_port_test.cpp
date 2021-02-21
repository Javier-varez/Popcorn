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

#include "popcorn/core/cortex-m_port.h"
#include "popcorn/core/cortex-m_registers.h"
#include "popcorn/core/syscall_idx.h"
#include "test/mock_kernel.h"
#include "test/mock_assert.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::StrEq;
using ::testing::_;
using ::testing::Ref;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::unique_ptr;
using std::make_unique;

using Popcorn::SyscallIdx;
using Popcorn::Priority;
using Popcorn::Lockable;
using Hw::auto_task_stack_frame;
using Hw::g_SCB;
using Hw::SCB_t;
using Hw::g_SysTick;
using Hw::SysTick_t;
using Hw::MCU;

namespace Hw {
class MCUWithLowLevelMock: public Hw::MCU {
 public:
  MOCK_METHOD(void, DisableInterruptsInternal, (), (const));
  MOCK_METHOD(void, EnableInterruptsInternal, (), (const));
};

void DestroyTaskVeneer();

class MCUTest: public ::testing::Test {
 private:
  void SetUp() override {
    mcu = make_unique<MCUWithLowLevelMock>();
    kernel = make_unique<StrictMock<MockKernel>>(mcu.get());
    g_SCB = &scb;
    g_SysTick = &systick;
    g_platform = &platform;
  }

  void TearDown() override {
  }

 protected:
  void HandleSVC(struct auto_task_stack_frame* frame) {
    mcu->HandleSVC(frame);
  }

  std::atomic_uint32_t& GetNestedInterruptLevel() {
    return mcu->m_nested_interrupt_level;
  }

  unique_ptr<StrictMock<MockKernel>> kernel;
  SCB_t scb;
  SysTick_t systick;
  unique_ptr<MCUWithLowLevelMock> mcu;
  StrictMock<MockPlatform> platform;
};

struct SVC_OP {
  uint8_t immediate;
  uint8_t opcode;

  explicit SVC_OP(uint8_t i) {
    immediate = i;
    opcode = 0xDF;
  }
};

TEST_F(MCUTest, HandleSVC_StartOS_Test) {
  SVC_OP StartOS_SVC(static_cast<uint8_t>(SyscallIdx::StartOS));
  struct auto_task_stack_frame frame;
  frame.pc = (uint32_t)(&StartOS_SVC) + sizeof(uint16_t);
  frame.xpsr = 0;

  EXPECT_CALL(*kernel, StartOS()).Times(1).RetiresOnSaturation();
  HandleSVC(&frame);
}

static void testfunc(void *arg) {}

struct CallStack {
  auto_task_stack_frame frame;
  uint32_t data[32];
};

TEST_F(MCUTest, HandleSVC_CreateTask_Test) {
  SVC_OP CreateTask_SVC(static_cast<uint8_t>(SyscallIdx::CreateTask));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&CreateTask_SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  void* arg = reinterpret_cast<void*>(0xF1F2F3F4);
  constexpr uint32_t stack_size = 123;
  enum Priority prio = Priority::Level_5;
  const char name[] = "FuncName";

  callStack.frame.r1 = (uint32_t)testfunc;
  callStack.frame.r2 = 0xF1F2F3F4;
  callStack.frame.r3 = (uint32_t)prio;
  callStack.data[0] = (uint32_t)name;
  callStack.data[1] = (uint32_t)stack_size;

  EXPECT_CALL(*kernel, CreateTask(testfunc, arg, prio, name, stack_size))
      .Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);

  // Now with aligned stack on entry
  callStack.frame.xpsr |= 1U << 9;

  callStack.data[1] = (uint32_t)name;
  callStack.data[2] = (uint32_t)stack_size;
  EXPECT_CALL(*kernel, CreateTask(testfunc, arg, prio, StrEq(name), stack_size))
      .Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Sleep_Test) {
  SVC_OP Sleep_SVC(static_cast<uint8_t>(SyscallIdx::Sleep));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&Sleep_SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  constexpr uint32_t sleep_time_ms = 1234;
  callStack.frame.r1 = sleep_time_ms;

  EXPECT_CALL(*kernel, Sleep(sleep_time_ms)).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_DestroyTask_Test) {
  SVC_OP SVC(static_cast<uint8_t>(SyscallIdx::DestroyTask));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  EXPECT_CALL(*kernel, DestroyTask()).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Yield_Test) {
  SVC_OP SVC(static_cast<uint8_t>(SyscallIdx::Yield));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  EXPECT_CALL(*kernel, Yield()).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Wait_Test) {
  SVC_OP SVC(static_cast<uint8_t>(SyscallIdx::Wait));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  Lockable lockable;
  callStack.frame.r1 = (uint32_t)&lockable;

  EXPECT_CALL(*kernel, Wait(Ref(lockable))).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_RegisterError_Test) {
  SVC_OP SVC(static_cast<uint8_t>(SyscallIdx::RegisterError));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  EXPECT_CALL(*kernel, RegisterError())
    .Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Unknown_Test) {
  SVC_OP SVC(0xFF);
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  EXPECT_CALL(*kernel, RegisterError())
    .Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Lock_Test) {
  SVC_OP SVC(static_cast<uint8_t>(SyscallIdx::Lock));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  Lockable lockable;
  callStack.frame.r1 = (uint32_t)&lockable;
  callStack.frame.r2 = (uint32_t)true;

  EXPECT_CALL(*kernel, Lock(Ref(lockable), true)).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);

  callStack.frame.r1 = (uint32_t)&lockable;
  callStack.frame.r2 = (uint32_t)false;

  EXPECT_CALL(*kernel, Lock(Ref(lockable), false)).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, EnableDisableInterrupts) {
  auto& nested_interrupt_level = GetNestedInterruptLevel();
  EXPECT_EQ(nested_interrupt_level, 0U);

  InSequence s;
  EXPECT_CALL(*mcu, DisableInterruptsInternal()).RetiresOnSaturation();
  mcu->DisableInterrupts();
  EXPECT_EQ(nested_interrupt_level, 1U);

  EXPECT_CALL(*mcu, DisableInterruptsInternal()).Times(0);
  mcu->DisableInterrupts();
  EXPECT_EQ(nested_interrupt_level, 2U);

  mcu->EnableInterrupts();
  EXPECT_EQ(nested_interrupt_level, 1U);

  EXPECT_CALL(*mcu, EnableInterruptsInternal());
  mcu->EnableInterrupts();
  EXPECT_EQ(nested_interrupt_level, 0U);
}

TEST_F(MCUTest, EnableInterruptsNotPreviouslyDisabled) {
  EXPECT_CALL(platform, Assert(_));
  mcu->EnableInterrupts();
}

TEST_F(MCUTest, Initialize) {
  mcu->Initialize();
  EXPECT_EQ(scb.SHP[Hw::SYSTICK_SHP_IDX], 0xFF);
  EXPECT_EQ(scb.SHP[Hw::PEND_SV_SHP_IDX], 0xFF);
  EXPECT_EQ(scb.SHP[Hw::SVC_CALL_SHP_IDX], 0x00);

  EXPECT_EQ(systick.LOAD, 71999U);
  EXPECT_EQ(systick.VAL, 0U);
  EXPECT_EQ(systick.CTRL, 7U);

  EXPECT_TRUE(scb.CCR & Hw::SCB_CCR_STKALIGN);
}

TEST_F(MCUTest, InitializeTask) {
  constexpr uint32_t kStackSize = 1024;
  uint8_t stack[kStackSize];
  Popcorn::task_func func = [](void* arg) { };
  uint8_t arg = 12;

  // Align stack to 8 bytes
  uintptr_t stack_base = reinterpret_cast<uintptr_t>(&stack) & 0xFFFFFFF8;
  uint8_t* stack_top = reinterpret_cast<uint8_t*>(stack_base + kStackSize);

  // Initialize the task
  uint8_t* stack_frame_ptr = mcu->InitializeTask(stack_top, func, &arg);
  auto* stack_frame = reinterpret_cast<task_stack_frame*>(stack_frame_ptr);

  const auto destroy_function = reinterpret_cast<uintptr_t>(Hw::DestroyTaskVeneer);
  const auto task_function = reinterpret_cast<uintptr_t>(func);
  const auto arg_ptr = reinterpret_cast<uintptr_t>(&arg);

  // Check the expectations
  ASSERT_EQ(stack_frame_ptr, stack_top - 68);
  ASSERT_EQ(stack_frame->manualsave.lr, EXC_RETURN_PSP_UNPRIV);
  ASSERT_EQ(stack_frame->autosave.lr, destroy_function);
  ASSERT_EQ(stack_frame->autosave.pc, task_function);
  ASSERT_EQ(stack_frame->autosave.r0, arg_ptr);
  ASSERT_EQ(stack_frame->autosave.xpsr, XPSR_INIT_VALUE);
}

}  // namespace Hw
