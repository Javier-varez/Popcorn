#ifndef MOCK_MEM_MANAGEMENT_H_
#define MOCK_MEM_MANAGEMENT_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using ::testing::StrictMock;

class MemManagement
{
public:
    virtual void* Malloc(std::size_t size) = 0;
    virtual void Free(void* ptr) = 0;
    virtual ~MemManagement() {}
};

class MockMemManagement: public MemManagement
{
public:
    MOCK_METHOD1(Malloc, void*(std::size_t));
    MOCK_METHOD1(Free, void(void*));
    virtual ~MockMemManagement() {}
};

extern StrictMock<MockMemManagement> *g_MockMemManagement;

#endif