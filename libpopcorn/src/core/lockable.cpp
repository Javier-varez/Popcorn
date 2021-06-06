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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "popcorn/core/lockable.h"
#include "popcorn/API/syscall.h"

namespace Popcorn {

void Lockable::Block() {
  Syscall::Instance().Wait(*this);
}

void Lockable::LockAcquired() {
  Syscall::Instance().Lock(*this, true);
}

void Lockable::LockReleased() {
  Syscall::Instance().Lock(*this, false);
}

void Lockable::SetBlockerTask(task_control_block* tcb) {
  m_blocker = tcb;
}

task_control_block* Lockable::GetBlockerTask() const {
  return m_blocker;
}

}  // namespace Popcorn
