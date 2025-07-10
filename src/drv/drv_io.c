#include <stdio.h>
#include "drv_led.h"

void drv_io_init(void)
{
		R_IOPORT_Open(g_ioport.p_ctrl, g_ioport.p_cfg);
}



