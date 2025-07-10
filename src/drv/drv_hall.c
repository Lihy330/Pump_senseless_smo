#include <stdio.h>
#include "drv_hall.h"

void drv_hall_init()
{
	R_GPT_Open(g_timer3.p_ctrl, g_timer3.p_cfg);
	R_GPT_Start(g_timer3.p_ctrl);

	R_GPT_Open(g_timer4.p_ctrl, g_timer4.p_cfg);
	R_GPT_Enable(g_timer4.p_ctrl);
	R_GPT_Start(g_timer4.p_ctrl);

	R_GPT_Open(g_timer5.p_ctrl, g_timer5.p_cfg);
	R_GPT_Enable(g_timer5.p_ctrl);
	R_GPT_Start(g_timer5.p_ctrl);
}

