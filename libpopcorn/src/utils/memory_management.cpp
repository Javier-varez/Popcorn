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

#include <stdlib.h>

#include "popcorn/utils/memory_management.h"
#include "popcorn/primitives/spinlock.h"
#include "popcorn/primitives/unique_lock.h"
#include "popcorn/platform.h"

using Popcorn::SpinLock;
using Popcorn::UniqueLock;

static SpinLock lock;

CLINKAGE void* OsMalloc(size_t size) {
  // TODO(javier_varez): Migrate to custom allocation scheme
  UniqueLock<SpinLock> l(lock);
  void* ptr = malloc(size);
  return ptr;
}

CLINKAGE void OsFree(void* ptr) {
  UniqueLock<SpinLock> l(lock);
  free(ptr);
}
