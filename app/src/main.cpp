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

#include <stm32f1xx_hal.h>

#include <cstring>
#include <cstdio>
#include <memory>

#include "popcorn/API/syscall.h"
#include "popcorn/primitives/mutex.h"
#include "popcorn/primitives/unique_lock.h"
#include "popcorn/core/kernel.h"

#include "postform/rtt_logger.h"
#include "postform/config.h"

using Popcorn::Mutex;
using Popcorn::UniqueLock;
using Popcorn::Syscall;
using Popcorn::Priority;

Postform::RttLogger logger;

namespace Postform {
uint64_t getGlobalTimestamp() {
  static std::atomic_uint32_t count;
  return count.fetch_add(1, std::memory_order_relaxed);
}
}  // namespace Postform

DECLARE_POSTFORM_CONFIG(.timestamp_frequency = 1);

void App_SysTick_Hook() {
  HAL_IncTick();
}

extern "C" void HardFault_Handler() {
  LOG_ERROR(&logger, "Hardfault handler was called!");

  auto CFSR = reinterpret_cast<volatile uint32_t *>(0xE000ED28);
  LOG_ERROR(&logger, "CFSR Value = %x", *CFSR);

  while (true) { }
}

void Popcorn::Kernel::TriggerSchedulerEntryHook() {
  // Performance is critical here. That is why
  // we access the registers directly
  constexpr uint32_t PIN_1 = 1;

  GPIOA->BSRR = 1 << PIN_1;  // GPIOA1 = 1
}

void Popcorn::Kernel::TriggerSchedulerExitHook() {
  // Performance is critical here. That is why
  // we access the registers directly
  constexpr uint32_t PIN_1 = 1;
  constexpr uint32_t CLEAR_OFFSET = 16;
  GPIOA->BSRR = 1 << (CLEAR_OFFSET + PIN_1);  // GPIOA1 = 0
}

struct TaskArgs {
  const char*     name;
  GPIO_TypeDef*   bank;
  uint16_t   pin;
  uint32_t   delay;
  uint32_t   stack_size;
};

static Mutex mutex;

static void gpio_task(void* arg) {
  TaskArgs* args = reinterpret_cast<TaskArgs*>(arg);
  LOG_INFO(&logger, "Gpio taks Bank %p, Pin %hu", args->bank, args->pin);

  if (args->bank == GPIOA) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  } else if (args->bank == GPIOC) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  }

  {
    UniqueLock<Mutex> l(mutex);
    GPIO_InitTypeDef gpio;
    gpio.Pin = args->pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(args->bank, &gpio);
  }

  while (true) {  // -V776
    Syscall::Instance().Sleep(args->delay);
    HAL_GPIO_TogglePin(args->bank, args->pin);
    LOG_INFO(&logger, "Toggling pin %p, %hu", args->bank, args->pin);
  }
}

static void ConfigureClk() {
  RCC_OscInitTypeDef oscConfig;
  oscConfig.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  oscConfig.HSEState = RCC_HSE_ON;
  oscConfig.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  oscConfig.LSEState = RCC_LSE_OFF;
  oscConfig.HSIState = RCC_HSI_OFF;
  oscConfig.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  oscConfig.LSIState = RCC_LSI_OFF;
  oscConfig.PLL.PLLState = RCC_PLL_ON;
  oscConfig.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  oscConfig.PLL.PLLMUL = RCC_PLL_MUL9;

  HAL_RCC_OscConfig(&oscConfig);

  RCC_ClkInitTypeDef clkInit;
  clkInit.ClockType = RCC_CLOCKTYPE_SYSCLK |
                      RCC_CLOCKTYPE_HCLK |
                      RCC_CLOCKTYPE_PCLK1 |
                      RCC_CLOCKTYPE_PCLK2;
  clkInit.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkInit.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkInit.APB1CLKDivider = RCC_HCLK_DIV2;
  clkInit.APB2CLKDivider = RCC_HCLK_DIV1;

  HAL_RCC_ClockConfig(&clkInit, FLASH_ACR_LATENCY_2);
}

void AteAssertFailed(std::uintptr_t PC) {
  LOG_ERROR(&logger, "ASSERT %x", PC);
}

void InitTask(void *args) {
  TaskArgs** taskArgs = reinterpret_cast<TaskArgs**>(args);
  Syscall& syscall = Syscall::Instance();
  LOG_INFO(&logger, "Running InitTask");

  {
    UniqueLock<Mutex> l(mutex);

    for (uint32_t i = 0; taskArgs[i] != nullptr; i++) {
      syscall.CreateTask(gpio_task, taskArgs[i], Priority::Level_0,
                         taskArgs[i]->name, taskArgs[i]->stack_size);
    }

    syscall.Sleep(1000);
  }
}

// HAL should not use the systick, It is used by the OS
CLINKAGE HAL_StatusTypeDef HAL_InitTick(uint32_t) {
  return HAL_OK;
}

int main() {
  LOG_INFO(&logger, "Popcorn is starting up!");

  struct TaskArgs args_task_1 = {
    "GPIO_C13",
    GPIOC,
    GPIO_PIN_13,
    1000,
    256
  };

  struct TaskArgs args_task_2 = {
    "GPIO_A0",
    GPIOA,
    GPIO_PIN_0,
    1500,
    256
  };

  struct TaskArgs* args[] = {
    &args_task_1,
    &args_task_2,
    nullptr
  };

  static constexpr uint32_t InitTaskStackSize = 512;

  HAL_Init();
  ConfigureClk();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef gpio;
  gpio.Pin = GPIO_PIN_1;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &gpio);

  auto& syscall = Syscall::Instance();
  syscall.CreateTask(InitTask, args, Priority::Level_0,
                                 "Init task", InitTaskStackSize);
  syscall.StartOS();
  return 0;
}
