#include "stm32f1xx_hal.h"

extern volatile int scheduler_active;

void HardFault_Handler()
{
	while (1);	
}

void MemManage_Handler()
{
	while (1);	
}

void BusFault_Handler()
{
	while (1);	
}

void UsageFault_Handler()
{
	while (1);	
}