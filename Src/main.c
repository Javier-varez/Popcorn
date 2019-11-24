#include "stm32f1xx_hal.h"

volatile int scheduler_active = 0;

#define MAX_TASKS 2
#define STACK_SIZE 10241
#define TASK_STACK_SIZE 1024

static volatile void* tasks_psp[MAX_TASKS];
static volatile int n_tasks = 0;
static volatile int current_task = 0;

typedef void (*task_func)();

static volatile uint8_t stack[STACK_SIZE] __attribute__ ((aligned(8)));

struct task_stack_frame
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t ret_addr;
	uint32_t xpsr;
} __attribute__((packed));

struct complete_task_stack_frame
{
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t lr;
	struct task_stack_frame autosave;
} __attribute__((packed));

void PendSV_Handler()
{
	asm volatile(
		"    mrs r0, psp                   \n"		
		"    isb                           \n"
		"    ldr r1, pxCurrentTask         \n"
		"    ldr r2, pxTasksPSP            \n"
		"    stmdb r0!, {r4-r11, r14}      \n"
		"    ldr r3, [r1]                  \n" // r3 = current_task
		"    str r0, [r2, r3, LSL #2]      \n" // tasks_psp[current_task] = psp
		"    eor r3, r3, #1                \n" // switch task
		"    str r3, [r1]                  \n" // current_task = (current_task + 1) % 2;
		"    ldr r0, [r2, r3, LSL #2]      \n" // r0 = tasks_psp[current_task]
		"    ldmia r0!, {r4-r11, r14}      \n"
		"    msr psp, r0                   \n"
		"    isb                           \n"
		"    bx lr                         \n"
		"pxCurrentTask: .word current_task \n"
		"pxTasksPSP: .word tasks_psp");
}

static void CreateTask(task_func func)
{
	if (n_tasks < MAX_TASKS)
	{
		n_tasks++;
		struct complete_task_stack_frame* task_psp = &stack[STACK_SIZE - 1 - TASK_STACK_SIZE * n_tasks - sizeof(struct complete_task_stack_frame)];
		task_psp->autosave.ret_addr = (uint32_t)func | 0x01;
		task_psp->autosave.xpsr = 1 << 24;
		task_psp->autosave.lr = 0xA5A5A5A5;
		task_psp->lr = 0xFFFFFFFD;
		tasks_psp[n_tasks - 1] = task_psp;
	}
}

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

static void StartScheduler()
{
	__asm volatile 	( " cpsid i " ::: "memory" );

	__NVIC_SetPriority(SysTick_IRQn, 0);
	__NVIC_SetPriority(PendSV_IRQn, 3);

	CreateTask(gpio_task1);
	CreateTask(gpio_task2);
	scheduler_active = 1;

	// Start executing first task
	__set_PSP(tasks_psp[0]);
	__asm volatile 	( " cpsie i " ::: "memory" );
	__set_CONTROL(0x03);
	gpio_task1();

	while (1);
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
	StartScheduler();
	return 0;
}