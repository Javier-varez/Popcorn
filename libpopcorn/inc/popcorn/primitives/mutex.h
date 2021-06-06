/*
 * This file is part of Popcorn
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

#ifndef POPCORN_PRIMITIVES_MUTEX_H_
#define POPCORN_PRIMITIVES_MUTEX_H_

#include <atomic>

#include "popcorn/core/lockable.h"

class MutexTest;

namespace Popcorn {
class Mutex: Lockable {
 public:
  Mutex();
  void Lock();
  void Unlock();

 private:
  std::atomic_flag m_held;
  friend MutexTest;
};
}  // namespace Popcorn

#endif  // POPCORN_PRIMITIVES_MUTEX_H_
