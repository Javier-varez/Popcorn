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

#include "Inc/cortex-m_port.h"
#include "Inc/cortex-m_registers.h"
#include "Inc/syscall_idx.h"
#include "Test/Inc/MockKernel.h"
#include "Test/Inc/MockAssert.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::StrEq;
using ::testing::_;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::unique_ptr;
using std::make_unique;

using OS::g_kernel;
using OS::SyscallIdx;
using OS::auto_task_stack_frame;
using OS::Priority;
using OS::Blockable;
using Hw::g_SCB;
using Hw::SCB_t;
using Hw::g_SysTick;
using Hw::SysTick_t;
using Hw::MCU;

class MCUWithLowLevelMock: public Hw::MCU {
 public:
  MOCK_METHOD(void, DisableInterruptsInternal, (), (const));
  MOCK_METHOD(void, EnableInterruptsInternal, (), (const));
};

class MCUTest: public ::testing::Test {
 private:
  void SetUp() override {
    mcu = make_unique<MCUWithLowLevelMock>();
    g_kernel = &kernel;
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
    return mcu->nested_interrupt_level;
  }

  StrictMock<MockKernel> kernel;
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

  EXPECT_CALL(kernel, StartOS()).Times(1).RetiresOnSaturation();
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

  EXPECT_CALL(kernel, CreateTask(testfunc, arg, prio, name, stack_size))
      .Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);

  // Now with aligned stack on entry
  callStack.frame.xpsr |= 1U << 9;

  callStack.data[1] = (uint32_t)name;
  callStack.data[2] = (uint32_t)stack_size;
  EXPECT_CALL(kernel, CreateTask(testfunc, arg, prio, StrEq(name), stack_size))
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

  EXPECT_CALL(kernel, Sleep(sleep_time_ms)).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_DestroyTask_Test) {
  SVC_OP SVC(static_cast<uint8_t>(SyscallIdx::DestroyTask));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  EXPECT_CALL(kernel, DestroyTask()).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Yield_Test) {
  SVC_OP SVC(static_cast<uint8_t>(SyscallIdx::Yield));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  EXPECT_CALL(kernel, Yield()).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Wait_Test) {
  SVC_OP SVC(static_cast<uint8_t>(SyscallIdx::Wait));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  Blockable blockable;
  callStack.frame.r1 = (uint32_t)&blockable;

  EXPECT_CALL(kernel, Wait(&blockable)).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_RegisterError_Test) {
  SVC_OP SVC(static_cast<uint8_t>(SyscallIdx::RegisterError));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  EXPECT_CALL(kernel, RegisterError(&callStack.frame))
    .Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Unknown_Test) {
  SVC_OP SVC(0xFF);
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  EXPECT_CALL(kernel, RegisterError(&callStack.frame))
    .Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Lock_Test) {
  SVC_OP SVC(static_cast<uint8_t>(SyscallIdx::Lock));
  struct CallStack callStack;
  callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
  callStack.frame.xpsr = 0;

  Blockable blockable;
  callStack.frame.r1 = (uint32_t)&blockable;
  callStack.frame.r2 = (uint32_t)true;

  EXPECT_CALL(kernel, Lock(&blockable, true)).Times(1).RetiresOnSaturation();
  HandleSVC(&callStack.frame);

  callStack.frame.r1 = (uint32_t)&blockable;
  callStack.frame.r2 = (uint32_t)false;

  EXPECT_CALL(kernel, Lock(&blockable, false)).Times(1).RetiresOnSaturation();
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

  EXPECT_CALL(platform, Assert(_, _, true)).RetiresOnSaturation();
  mcu->EnableInterrupts();
  EXPECT_EQ(nested_interrupt_level, 1U);

  EXPECT_CALL(platform, Assert(_, _, true)).RetiresOnSaturation();
  EXPECT_CALL(*mcu, EnableInterruptsInternal());
  mcu->EnableInterrupts();
  EXPECT_EQ(nested_interrupt_level, 0U);
}

TEST_F(MCUTest, EnableInterruptsNotPreviouslyDisabled) {
  EXPECT_CALL(platform, Assert(_, _, false));
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
