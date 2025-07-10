#include <stdio.h>
#include "drv_led.h"

#define LED1_CTRL_ON() R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_08, BSP_IO_LEVEL_LOW);
#define LED2_CTRL_ON() R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_09, BSP_IO_LEVEL_LOW);
#define LED3_CTRL_ON() R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_10, BSP_IO_LEVEL_LOW);
#define LED4_CTRL_ON() R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_11, BSP_IO_LEVEL_LOW);

#define LED1_CTRL_OFF() R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_08, BSP_IO_LEVEL_HIGH);
#define LED2_CTRL_OFF() R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_09, BSP_IO_LEVEL_HIGH);
#define LED3_CTRL_OFF() R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_10, BSP_IO_LEVEL_HIGH);
#define LED4_CTRL_OFF() R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_11, BSP_IO_LEVEL_HIGH);


void drv_led_init(void)
{
	LED1_CTRL_ON();
	LED2_CTRL_ON();
	LED3_CTRL_ON();
	LED4_CTRL_ON();
}

void drv_led_ctrl(led_num_e num, uint8_t status)
{
	switch(num)
	{
		case LED1:
			switch(status)
			{
				case LED_ON:
					LED1_CTRL_ON();
				break;
				case LED_OFF:
					LED1_CTRL_OFF();
				break;
			}
		break;
		case LED2:
			switch(status)
			{
				case LED_ON:
					LED2_CTRL_ON();
				break;
				case LED_OFF:
					LED2_CTRL_OFF();
				break;
			}
		break;
		case LED3:
			switch(status)
			{
				case LED_ON:
					LED3_CTRL_ON();
				break;
				case LED_OFF:
					LED3_CTRL_OFF();
				break;
			}
		break;		
		case LED4:
			switch(status)
			{
				case LED_ON:
					LED4_CTRL_ON();
				break;
				case LED_OFF:
					LED4_CTRL_OFF();
				break;
			}
		break;	
	}
}

