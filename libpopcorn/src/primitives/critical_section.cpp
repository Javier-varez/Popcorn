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

#include "popcorn/primitives/critical_section.h"

#include "popcorn/core/cortex-m_port.h"

namespace Hw {
  extern MCU* g_mcu;
}  // namespace Hw

namespace Popcorn {
CriticalSection::CriticalSection() {
  if (Hw::g_mcu) {
    Hw::g_mcu->DisableInterrupts();
  }
}

CriticalSection::~CriticalSection() {
  if (Hw::g_mcu) {
    Hw::g_mcu->EnableInterrupts();
  }
}
}  // namespace Popcorn
