#ifndef MOCK_KERNEL_H_
#define MOCK_KERNEL_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "kernel.h"

namespace OS
{
    class MockKernel : public Kernel
    {
    public:
        MockKernel() = default;
        MOCK_METHOD0(StartOS, void());
        MOCK_METHOD4(CreateTask, void(task_func func, uintptr_t arg, enum OS::Priority priority, const char* name));
        MOCK_METHOD1(Sleep, void(std::uint32_t ticks));
        MOCK_METHOD0(DestroyTask, void());
        MOCK_METHOD0(Yield, void());
        MOCK_METHOD1(Wait, void(const Blockable* Blockable));
        MOCK_METHOD1(RegisterError, void(struct auto_task_stack_frame* args));
        MOCK_METHOD0(GetTicks, std::uint64_t());
        MOCK_METHOD0(TriggerScheduler, void());
        MOCK_METHOD0(HandleTick, void());        
    };
} // namespace Hw

#endif