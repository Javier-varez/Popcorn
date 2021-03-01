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

#ifndef POPCORN_CORE_CORTEX_M_REGISTERS_H_
#define POPCORN_CORE_CORTEX_M_REGISTERS_H_

#include <cstdint>

namespace Hw {
constexpr std::uint32_t SCB_CCR_STKALIGN = 1U << 9;
constexpr std::uint32_t SCB_ICSR_PENDSVSET = 1U << 28;

constexpr std::uint32_t SYSTICK_SHP_IDX = 11;
constexpr std::uint32_t PEND_SV_SHP_IDX = 10;
constexpr std::uint32_t SVC_CALL_SHP_IDX = 7;

constexpr std::uint32_t SCB_ADDR = 0xE000ED00UL;
struct SCB_t {
  std::uint32_t CPUID;
  std::uint32_t ICSR;
  std::uint32_t VTOR;
  std::uint32_t AIRCR;
  std::uint32_t SCR;
  std::uint32_t CCR;
  std::uint8_t  SHP[12U];
  std::uint32_t SHCSR;
  std::uint32_t CFSR;
  std::uint32_t HFSR;
  std::uint32_t DFSR;
  std::uint32_t MMFAR;
  std::uint32_t BFAR;
  std::uint32_t AFSR;
  std::uint32_t PFR[2U];
  std::uint32_t DFR;
  std::uint32_t ADR;
  std::uint32_t MMFR[4U];
  std::uint32_t ISAR[5U];
  std::uint32_t RESERVED0[5U];
  std::uint32_t CPACR;
};
extern volatile SCB_t *g_SCB;

constexpr std::uint32_t SYSTICK_ADDR = 0xE000E010UL;
struct SysTick_t {
  std::uint32_t CTRL;
  std::uint32_t LOAD;
  std::uint32_t VAL;
  std::uint32_t CALIB;
};
extern volatile SysTick_t *g_SysTick;

constexpr std::uint32_t SysTick_Ctrl_CountFlag        = (1UL << 16U);
constexpr std::uint32_t SysTick_Ctrl_ClkSource        = (1UL <<  2U);
constexpr std::uint32_t SysTick_Ctrl_TickInt          = (1UL <<  1U);
constexpr std::uint32_t SysTick_Ctrl_Enable           = (1UL <<  0U);

}  // namespace Hw

#endif  // POPCORN_CORE_CORTEX_M_REGISTERS_H_
