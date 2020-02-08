#include <memory>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "cortex-m_port.h"
#include "cortex-m_registers.h"
#include "syscall_idx.h"
#include "MockKernel.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::_;


class MCUTest: public ::testing::Test
{
private:
    void SetUp() override
    {
        mcu = std::make_unique<Hw::MCU>();
        OS::g_kernel = &kernel;
        Hw::g_SCB = &scb;
    }

    void TearDown() override
    {

    }

protected:
    void HandleSVC(struct OS::auto_task_stack_frame* frame)
    {
        mcu->HandleSVC(frame);
    }

    StrictMock<OS::MockKernel> kernel;
    Hw::SCB_t scb;
    std::unique_ptr<Hw::MCU> mcu;
};

struct SVC_OP
{
    uint8_t immediate;
    uint8_t opcode;
    
    SVC_OP(uint8_t i)
    {
        immediate = i;
        opcode = 0xDF;
    }
};

TEST_F(MCUTest, HandleSVC_StartOS_Test)
{
    SVC_OP StartOS_SVC(static_cast<uint8_t>(OS::SyscallIdx::StartOS));
    struct OS::auto_task_stack_frame frame;
    frame.pc = (uint32_t)(&StartOS_SVC) + sizeof(uint16_t);
    frame.xpsr = 0;

    EXPECT_CALL(kernel, StartOS()).Times(1).RetiresOnSaturation();
    HandleSVC(&frame);
}

static void testfunc(void *arg) {}

struct CallStack
{
    OS::auto_task_stack_frame frame;
    uint32_t data[32];
};

TEST_F(MCUTest, HandleSVC_CreateTask_Test)
{
    SVC_OP CreateTask_SVC(static_cast<uint8_t>(OS::SyscallIdx::CreateTask));
    struct CallStack callStack;
    callStack.frame.pc = (uint32_t)(&CreateTask_SVC) + sizeof(uint16_t);
    callStack.frame.xpsr = 0;

    uint32_t arg = 0xF1F2F3F4;
    enum OS::Priority prio = OS::Priority::Level_5;
    const char name[] = "FuncName";

    callStack.frame.r1 = (uint32_t)testfunc;
    callStack.frame.r2 = arg;
    callStack.frame.r3 = (uint32_t)prio;
    callStack.data[0] = (uint32_t)name;

    EXPECT_CALL(kernel, CreateTask(testfunc, arg, prio, name)).Times(1).RetiresOnSaturation();
    HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Sleep_Test)
{
    SVC_OP Sleep_SVC(static_cast<uint8_t>(OS::SyscallIdx::Sleep));
    struct CallStack callStack;
    callStack.frame.pc = (uint32_t)(&Sleep_SVC) + sizeof(uint16_t);
    callStack.frame.xpsr = 0;

    callStack.frame.r1 = 1234;

    EXPECT_CALL(kernel, Sleep(1234)).Times(1).RetiresOnSaturation();
    HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_DestroyTask_Test)
{
    SVC_OP SVC(static_cast<uint8_t>(OS::SyscallIdx::DestroyTask));
    struct CallStack callStack;
    callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
    callStack.frame.xpsr = 0;

    EXPECT_CALL(kernel, DestroyTask()).Times(1).RetiresOnSaturation();
    HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Yield_Test)
{
    SVC_OP SVC(static_cast<uint8_t>(OS::SyscallIdx::Yield));
    struct CallStack callStack;
    callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
    callStack.frame.xpsr = 0;

    EXPECT_CALL(kernel, Yield()).Times(1).RetiresOnSaturation();
    HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Wait_Test)
{
    SVC_OP SVC(static_cast<uint8_t>(OS::SyscallIdx::Wait));
    struct CallStack callStack;
    callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
    callStack.frame.xpsr = 0;

    OS::Blockable blockable;
    callStack.frame.r1 = (uint32_t)&blockable;

    EXPECT_CALL(kernel, Wait(&blockable)).Times(1).RetiresOnSaturation();
    HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_RegisterError_Test)
{
    SVC_OP SVC(static_cast<uint8_t>(OS::SyscallIdx::RegisterError));
    struct CallStack callStack;
    callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
    callStack.frame.xpsr = 0;

    EXPECT_CALL(kernel, RegisterError(&callStack.frame)).Times(1).RetiresOnSaturation();
    HandleSVC(&callStack.frame);
}

TEST_F(MCUTest, HandleSVC_Unknown_Test)
{
    SVC_OP SVC(0xFF);
    struct CallStack callStack;
    callStack.frame.pc = (uint32_t)(&SVC) + sizeof(uint16_t);
    callStack.frame.xpsr = 0;

    EXPECT_CALL(kernel, RegisterError(&callStack.frame)).Times(1).RetiresOnSaturation();
    HandleSVC(&callStack.frame);
}
