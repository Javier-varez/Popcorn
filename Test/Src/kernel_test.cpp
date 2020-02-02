#include <memory>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MockMCU.h"
#include "kernel.h"
#include "syscall_idx.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::_;

class KernelTest: public ::testing::Test
{
private:
    void SetUp() override
    {
        Hw::g_mcu = &mcu;
        kernel = std::make_unique<OS::Kernel>();
    }

    void TearDown() override
    {

    }

protected:
    LinkedList_t* GetTaskList()
    {
        return kernel->task_list;
    }

    struct OS::task_control_block* GetCurrentTask()
    {
        return kernel->current_task;
    }

    std::uint64_t GetTicks()
    {
        return kernel->ticks;
    }

    void SetTicks(std::uint64_t ticks)
    {
        kernel->ticks = ticks;
    }

    void HandleTick()
    {
        kernel->HandleTick();
    }

    void TriggerScheduler()
    {
        kernel->TriggerScheduler();
    }

    void SetCurrentTask(OS::task_control_block* tcb)
    {
        kernel->current_task = tcb;
    }

    static void TaskFunction(void* arg)
    {

    }

    StrictMock<Hw::MockMCU> mcu;
    std::unique_ptr<OS::Kernel> kernel;
};

void IdleTask(void *arg);
void DestroyTaskVeneer();

TEST_F(KernelTest, StackPtrIsFirstFieldInTCB)
{
    // Assembly code relies on the stack_ptr being at
    // offset 0 from the task_control block
    struct OS::task_control_block tcb;
    ASSERT_EQ((void*)&tcb, (void*)&tcb.stack_ptr);
}

TEST_F(KernelTest, StartOS_Test)
{
    EXPECT_CALL(mcu, Initialize()).Times(1);
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1);

    kernel->StartOS();

    int i = 0;
    struct OS::task_control_block* tcb = nullptr;
    LinkedList_WalkEntry(GetTaskList(), tcb, list)
    {
        ASSERT_NE(tcb, nullptr);
        ASSERT_STREQ(tcb->name, "Idle");
        ASSERT_EQ(tcb->state, OS::task_state::RUNNABLE);
        ASSERT_EQ(tcb->priority, OS::Priority::IDLE);
        ASSERT_EQ(tcb->arg, 0U);
        ASSERT_EQ(tcb->func, &IdleTask);
        ASSERT_NE(tcb->stack_base, 0U);
        ASSERT_NE(tcb->stack_ptr, 0U);
        ASSERT_GT(tcb->stack_ptr, tcb->stack_base);

        OS::task_stack_frame* stack_frame =
            reinterpret_cast<OS::task_stack_frame*>(tcb->stack_ptr);

        ASSERT_EQ(stack_frame->manualsave.lr, EXC_RETURN_PSP_UNPRIV);
        ASSERT_EQ(stack_frame->autosave.lr, reinterpret_cast<uintptr_t>(DestroyTaskVeneer));
        ASSERT_EQ(stack_frame->autosave.pc, reinterpret_cast<uintptr_t>(&IdleTask));
        ASSERT_EQ(stack_frame->autosave.r0, 0U);
        ASSERT_EQ(stack_frame->autosave.xpsr, XPSR_INIT_VALUE);
        i++;
    }
    ASSERT_EQ(i, 1);
}

TEST_F(KernelTest, Yield_Test)
{
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1);
    kernel->Yield();
}

TEST_F(KernelTest, CreateTask_Test)
{
    std::uint32_t args[] = { 128, 125 };
    kernel->CreateTask(TaskFunction, reinterpret_cast<std::uintptr_t>(&args[0]), OS::Priority::Level_3, "TestTask1");
    kernel->CreateTask(TaskFunction, reinterpret_cast<std::uintptr_t>(&args[1]), OS::Priority::Level_7, "TestTask2");

    int i = 0;
    struct OS::task_control_block* tcb = nullptr;
    LinkedList_WalkEntry(GetTaskList(), tcb, list)
    {
        ASSERT_NE(tcb, nullptr);
        ASSERT_EQ(tcb->state, OS::task_state::RUNNABLE);
        ASSERT_EQ(tcb->func, &TaskFunction);
        ASSERT_NE(tcb->stack_base, 0U);
        ASSERT_NE(tcb->stack_ptr, 0U);
        ASSERT_GT(tcb->stack_ptr, tcb->stack_base);

        OS::task_stack_frame* stack_frame =
            reinterpret_cast<OS::task_stack_frame*>(tcb->stack_ptr);

        ASSERT_EQ(stack_frame->manualsave.lr, EXC_RETURN_PSP_UNPRIV);
        ASSERT_EQ(stack_frame->autosave.lr, reinterpret_cast<uintptr_t>(DestroyTaskVeneer));
        ASSERT_EQ(stack_frame->autosave.pc, reinterpret_cast<uintptr_t>(&TaskFunction));
        ASSERT_EQ(stack_frame->autosave.xpsr, XPSR_INIT_VALUE);

        switch (i)
        {
        case 0:
            ASSERT_STREQ(tcb->name, "TestTask1");
            ASSERT_EQ(tcb->priority, OS::Priority::Level_3);
            ASSERT_EQ(tcb->arg, reinterpret_cast<std::uintptr_t>(&args[0]));
            ASSERT_EQ(stack_frame->autosave.r0, reinterpret_cast<std::uint32_t>(&args[0]));
            break;
        case 1:
            ASSERT_STREQ(tcb->name, "TestTask2");
            ASSERT_EQ(tcb->priority, OS::Priority::Level_7);
            ASSERT_EQ(tcb->arg, reinterpret_cast<std::uintptr_t>(&args[1]));
            ASSERT_EQ(stack_frame->autosave.r0, reinterpret_cast<std::uint32_t>(&args[1]));
            break;

        default:
            FAIL();
            break;
        }
        i++;
    }
    ASSERT_EQ(i, 2);
}

TEST_F(KernelTest, DestroyTask_Test)
{
    std::uint32_t args[] = { 128, 125 };
    kernel->CreateTask(TaskFunction, reinterpret_cast<std::uintptr_t>(&args[0]), OS::Priority::Level_3, "TestTask1");
    kernel->CreateTask(TaskFunction, reinterpret_cast<std::uintptr_t>(&args[1]), OS::Priority::Level_7, "TestTask2");

    // Set first task as current task
    SetCurrentTask(CONTAINER_OF(GetTaskList(), OS::task_control_block, list));

    EXPECT_CALL(mcu, TriggerPendSV()).Times(1);

    kernel->DestroyTask();

    int i = 0;
    struct OS::task_control_block* tcb = nullptr;
    LinkedList_WalkEntry(GetTaskList(), tcb, list)
    {
        ASSERT_NE(tcb, nullptr);
        ASSERT_EQ(tcb->state, OS::task_state::RUNNABLE);
        ASSERT_EQ(tcb->func, &TaskFunction);
        ASSERT_NE(tcb->stack_base, 0U);
        ASSERT_NE(tcb->stack_ptr, 0U);
        ASSERT_GT(tcb->stack_ptr, tcb->stack_base);

        OS::task_stack_frame* stack_frame =
            reinterpret_cast<OS::task_stack_frame*>(tcb->stack_ptr);

        ASSERT_EQ(stack_frame->manualsave.lr, EXC_RETURN_PSP_UNPRIV);
        ASSERT_EQ(stack_frame->autosave.lr, reinterpret_cast<uintptr_t>(DestroyTaskVeneer));
        ASSERT_EQ(stack_frame->autosave.pc, reinterpret_cast<uintptr_t>(&TaskFunction));
        ASSERT_EQ(stack_frame->autosave.xpsr, XPSR_INIT_VALUE);

        switch (i)
        {
        case 0:
            ASSERT_STREQ(tcb->name, "TestTask2");
            ASSERT_EQ(tcb->priority, OS::Priority::Level_7);
            ASSERT_EQ(tcb->arg, reinterpret_cast<std::uintptr_t>(&args[1]));
            ASSERT_EQ(stack_frame->autosave.r0, reinterpret_cast<std::uint32_t>(&args[1]));
            break;

        default:
            FAIL();
            break;
        }
        i++;
    }
    ASSERT_EQ(i, 1);
}

TEST_F(KernelTest, Ticks_Test)
{
    EXPECT_EQ(kernel->GetTicks(), 0ULL);
    EXPECT_EQ(GetTicks(), 0ULL);

    for (std::uint64_t i = 0; i < 200; i++)
    {
        EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
        HandleTick();
        EXPECT_EQ(kernel->GetTicks(), i+1);
        EXPECT_EQ(GetTicks(), i+1);
    }

}

TEST_F(KernelTest, TriggerScheduler_Test)
{
    EXPECT_CALL(mcu, Initialize()).Times(1);
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1);

    kernel->CreateTask(TaskFunction, 0U, OS::Priority::Level_3, "TestTask1");
    kernel->StartOS();

    EXPECT_EQ(GetCurrentTask(), nullptr);

    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask1");
}

TEST_F(KernelTest, Sleep_Test)
{
    constexpr std::uint64_t SLEEP_TICKS = 1242;
    SetTicks(0xFFFFFFFFULL);

    EXPECT_CALL(mcu, Initialize()).Times(1).RetiresOnSaturation();
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();

    kernel->CreateTask(TaskFunction, 0U, OS::Priority::Level_3, "TestTask1");
    kernel->StartOS();

    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask1");

    EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
    kernel->Sleep(SLEEP_TICKS);

    TriggerScheduler();

    for (std::uint32_t i = 0; i < SLEEP_TICKS; i++)
    {
        EXPECT_NE(GetCurrentTask(), nullptr);
        EXPECT_STREQ(GetCurrentTask()->name, "Idle");
        EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
        HandleTick();
        TriggerScheduler();
    }

    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask1");
}

namespace 
{
    class FakeBlock: public OS::Blockable
    {
    public:
        bool IsBlocked() const override
        {
            return m_blocked;
        }

        void Lock()
        {
            m_blocked = true;
            Block();
        }

        void Unlock()
        {
            m_blocked = false;
        }
    private:
        bool m_blocked = false;
    };
}

TEST_F(KernelTest, Wait_Test)
{
    FakeBlock block;

    EXPECT_CALL(mcu, Initialize()).Times(1).RetiresOnSaturation();
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();

    kernel->CreateTask(TaskFunction, 0U, OS::Priority::Level_2, "TestTask1");
    kernel->StartOS();

    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask1");

    kernel->CreateTask(TaskFunction, 0U, OS::Priority::Level_3, "TestTask2");

    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask2");

    EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Wait));
    block.Lock();
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
    kernel->Wait(&block);

    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask1");

    block.Unlock();

    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask2");
}

TEST_F(KernelTest, Priority_Test)
{
    FakeBlock block;

    EXPECT_CALL(mcu, Initialize()).Times(1).RetiresOnSaturation();
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();

    kernel->CreateTask(TaskFunction, 0U, OS::Priority::Level_1, "TestTask1");
    kernel->StartOS();

    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask1");

    kernel->CreateTask(TaskFunction, 0U, OS::Priority::Level_3, "TestTask2");

    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask2");

    EXPECT_CALL(mcu, SupervisorCall(OS::SyscallIdx::Wait));
    block.Lock();
    EXPECT_CALL(mcu, TriggerPendSV()).Times(1).RetiresOnSaturation();
    kernel->Wait(&block);

    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask1");

    kernel->CreateTask(TaskFunction, 0U, OS::Priority::Level_2, "TestTask3");
    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask3");

    block.Unlock();

    TriggerScheduler();
    EXPECT_NE(GetCurrentTask(), nullptr);
    EXPECT_STREQ(GetCurrentTask()->name, "TestTask2");
}
