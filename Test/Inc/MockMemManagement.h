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

#ifndef TEST_INC_MOCKMEMMANAGEMENT_H_
#define TEST_INC_MOCKMEMMANAGEMENT_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class MemManagement {
 public:
  virtual void* Malloc(std::size_t size) = 0;
  virtual void Free(void* ptr) = 0;
  virtual ~MemManagement() {}
};

class MockMemManagement: public MemManagement {
 public:
  MOCK_METHOD1(Malloc, void*(std::size_t));
  MOCK_METHOD1(Free, void(void*));
  virtual ~MockMemManagement() {}
};

extern ::testing::StrictMock<MockMemManagement> *g_MockMemManagement;

#endif  // TEST_INC_MOCKMEMMANAGEMENT_H_
