#include "drv_systick.h"
#include <stdio.h>

volatile uint32_t systick_count;

void drv_systick_init(void)
{
		// Enter interrupt fuction once per 1ms.
		SysTick_Config(SystemCoreClock / 1000);
}

uint32_t drv_systick_time_get(void)
{
		return systick_count;
}

void drv_delay_ms(uint32_t time)
{
		uint32_t temp = 0;
		temp = systick_count + time;
		while (systick_count <= temp);
}

void SysTick_Handler(void)
{
		systick_count ++ ;
}





