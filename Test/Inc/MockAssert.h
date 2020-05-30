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

#ifndef TEST_INC_MOCKASSERT_H_
#define TEST_INC_MOCKASSERT_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class Platform {
 public:
  virtual void Assert(const char*, unsigned int, bool) = 0;
};

class MockPlatform : public Platform {
 public:
  MOCK_METHOD(void, Assert, (const char*, unsigned int, bool));
};

extern Platform* g_platform;

#endif  // TEST_INC_MOCKASSERT_H_
