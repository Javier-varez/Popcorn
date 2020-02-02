#ifndef MOCK_MCU_H_
#define MOCK_MCU_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "cortex-m_port.h"

namespace Hw
{
    class MockMCU : public MCU
    {
    public:
        MockMCU() = default;
        MOCK_METHOD0(Initialize, void());
        MOCK_METHOD0(TriggerPendSV, void());
        MOCK_METHOD1(SupervisorCall, void(OS::SyscallIdx));
    };
} // namespace Hw

#endif