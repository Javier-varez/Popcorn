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

#include <stm32f1xx_hal.h>
#include "Inc/syscall.h"
#include "Inc/mutex.h"
#include "Inc/unique_lock.h"
#include "Inc/cortex-m_port.h"

void App_SysTick_Hook() {
    HAL_IncTick();
}

void OS::Kernel::TriggerSchedulerEntryHook() {
    // Performance is critical here. That is why
    // we access the registers directly
    constexpr std::uint32_t PIN_1 = 1;
    constexpr std::uint32_t SET_OFFSET = 0;

    GPIOA->BSRR = 1 << (SET_OFFSET + PIN_1);  // GPIOA1 = 1
}

void OS::Kernel::TriggerSchedulerExitHook() {
    // Performance is critical here. That is why
    // we access the registers directly
    constexpr std::uint32_t PIN_1 = 1;
    constexpr std::uint32_t CLEAR_OFFSET = 16;
    GPIOA->BSRR = 1 << (CLEAR_OFFSET + PIN_1);  // GPIOA1 = 0
}

struct TaskArgs {
    const char*     name;
    GPIO_TypeDef*   bank;
    std::uint16_t   pin;
    std::uint32_t   delay;
    std::uint32_t   stack_size;
};

static OS::Mutex mutex;

static void gpio_task(void* arg) {
    struct TaskArgs* args = reinterpret_cast<struct TaskArgs*>(arg);

    if (args->bank == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (args->bank == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }

    {
        OS::UniqueLock<OS::Mutex> l(mutex);
        GPIO_InitTypeDef gpio;
        gpio.Pin = args->pin;
        gpio.Mode = GPIO_MODE_OUTPUT_PP;
        gpio.Pull = GPIO_NOPULL;
        gpio.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(args->bank, &gpio);
    }

    while (1) {
        OS::Syscall::Instance().Sleep(args->delay);
        HAL_GPIO_TogglePin(args->bank, args->pin);
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
    clkInit.ClockType =
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;
    clkInit.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clkInit.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clkInit.APB1CLKDivider = RCC_HCLK_DIV2;
    clkInit.APB2CLKDivider = RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&clkInit, FLASH_ACR_LATENCY_2);
}

void InitTask(void *args) {
    struct TaskArgs** taskArgs = (struct TaskArgs**)(args);

    {
        OS::UniqueLock<OS::Mutex> l(mutex);

        for (uint32_t i = 0; taskArgs[i] != nullptr; i++) {
            OS::Syscall::Instance().CreateTask(
                gpio_task,
                reinterpret_cast<uintptr_t>(taskArgs[i]),
                OS::Priority::Level_0,
                taskArgs[i]->name,
                taskArgs[i]->stack_size);
        }

        OS::Syscall::Instance().Sleep(1000);
    }
}

int main() {
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

    static constexpr std::uint32_t InitTaskStackSize = 512;

    HAL_Init();
    ConfigureClk();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpio;
    gpio.Pin = GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    OS::Syscall::Instance().CreateTask(
        InitTask,
        reinterpret_cast<uintptr_t>(args),
        OS::Priority::Level_0,
        "Init task",
        InitTaskStackSize);
    OS::Syscall::Instance().StartOS();
    return 0;
}
