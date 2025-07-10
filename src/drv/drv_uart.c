#include <stdio.h>
#include "drv_uart.h"
#include <string.h>


void drv_uart_init(void)
{
	// open uart0
	fsp_err_t ret_err = R_SCI_B_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
	assert(ret_err == FSP_SUCCESS);
}

void drv_uart_send_data(uint8_t *data, uint32_t len)
{	
	R_SCI_B_UART_Write(&g_uart0_ctrl, data, len);
	FSP_HARDWARE_REGISTER_WAIT(g_uart0_ctrl.p_reg->CSR_b.TEND, 1U);
}


int fputc(int ch, FILE *f)
{
	(void)f;
	R_SCI_B_UART_Write(g_uart0.p_ctrl, (uint8_t*)&ch, 1U);
	FSP_HARDWARE_REGISTER_WAIT(g_uart0_ctrl.p_reg->CSR_b.TEND, 1U);
	return ch;
}
