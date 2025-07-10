#include <stdio.h>
#include "drv_uart_cb.h"

void uart0_callback(uart_callback_args_t * p_args)
{
		uint8_t data = 0;
		switch(p_args->event)
		{
			case UART_EVENT_RX_CHAR:
				data = (uint8_t)p_args->data;
				R_SCI_B_UART_Write(g_uart0.p_ctrl, &data, 1U);
				FSP_HARDWARE_REGISTER_WAIT(g_uart0_ctrl.p_reg->CSR_b.TEND, 1U);
			break;
			default:
			break;
		}
}

