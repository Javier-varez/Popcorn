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

#include "test/mock_mcu.h"

namespace Hw {
MockSVC *g_svc = nullptr;
// Use this ASM symbol to force the linker to load symbols from this translation unit.
// Otherwise, since all of these symbols are already defined outside (even if weak)
// they are ignored.
volatile uint32_t dummy_asm_symbol;
}  // namespace HW

void _svc_call(Popcorn::SyscallIdx id) {
  if (Hw::g_svc) {
    Hw::g_svc->SupervisorCall(id);
  }
}
