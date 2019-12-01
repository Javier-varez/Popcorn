#include "stm32f1xx_hal.h"

extern volatile int scheduler_active;

void HardFault_Handler()
{
	while (1);	
}