#include "stm32f1xx_hal.h"
#include "os.h"

static void gpio_task1()
{
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef gpio;
	gpio.Pin = GPIO_PIN_13;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &gpio);

	while (1)
	{
		HAL_Delay(1000);
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	}
}

static void gpio_task2()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpio;
	gpio.Pin = GPIO_PIN_0;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio);

	while (1)
	{
		HAL_Delay(1500);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
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

int main()
{
	HAL_Init();
	ConfigureClk();

	CreateTask(gpio_task1);
	CreateTask(gpio_task2);

	StartOS();
	return 0;
}
