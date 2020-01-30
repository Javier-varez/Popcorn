#include "stm32f1xx_hal.h"
#include "os.h"
#include "mutex.h"
#include "unique_lock.h"

void App_SysTick_Hook()
{
    HAL_IncTick();
}

struct TaskArgs
{
    GPIO_TypeDef*   bank;
    uint16_t        pin;
    uint32_t        delay;
};

static OS::Mutex mutex;

static void gpio_task(void* arg)
{
    struct TaskArgs* args = reinterpret_cast<struct TaskArgs*>(arg);
    
    if (args->bank == GPIOA)
        __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (args->bank == GPIOC)
        __HAL_RCC_GPIOC_CLK_ENABLE();

    {
        OS::UniqueLock<OS::Mutex> l(mutex);
        GPIO_InitTypeDef gpio;
        gpio.Pin = args->pin;
        gpio.Mode = GPIO_MODE_OUTPUT_PP;
        gpio.Pull = GPIO_NOPULL;
        gpio.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(args->bank, &gpio);
    }

    while (1)
    {
        OS::Scheduler::Sleep(args->delay);
        HAL_GPIO_TogglePin(args->bank, args->pin);
    }
}

static void ConfigureClk()
{
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

void InitTask(void *args)
{
    static struct TaskArgs args_task_1 = 
    {
        GPIOC,
        GPIO_PIN_13,
        1000
    };

    static struct TaskArgs args_task_2 = 
    {
        GPIOA,
        GPIO_PIN_0,
        1500
    };

    {
        OS::UniqueLock<OS::Mutex> l(mutex);

        OS::Scheduler::CreateTask(gpio_task, (uintptr_t)&args_task_1, OS::Scheduler::TASK_PRIO_0, "GPIOC_13");
        OS::Scheduler::CreateTask(gpio_task, (uintptr_t)&args_task_2, OS::Scheduler::TASK_PRIO_0, "GPIOA_0");

        OS::Scheduler::Sleep(1000);
    }
}

int main()
{
    HAL_Init();
    ConfigureClk();

    OS::Scheduler::CreateTask(InitTask, (uintptr_t)NULL, OS::Scheduler::TASK_PRIO_0, "Init task");
    OS::Scheduler::StartOS();
    return 0;
}
