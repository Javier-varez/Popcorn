/*
 * This file is part of the Popcorn
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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "popcorn/primitives/spinlock.h"
#include "popcorn/API/syscall.h"

namespace Popcorn {
SpinLock::SpinLock() {
  m_held.clear();
}

void SpinLock::Lock() {
  bool was_already_held;
  do {
    was_already_held = m_held.test_and_set();
  } while (was_already_held);
}

void SpinLock::Unlock() {
  m_held.clear();
}
}  // namespace Popcorn
