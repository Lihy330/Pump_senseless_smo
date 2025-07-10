#include <stdio.h>
#include "drv_dac.h"

void drv_dac_init()
{
		R_DAC_Open(g_dac0.p_ctrl, g_dac0.p_cfg);
		R_DAC_Open(g_dac1.p_ctrl, g_dac1.p_cfg);
		R_DAC_Open(g_dac2.p_ctrl, g_dac2.p_cfg);
		R_DAC_Open(g_dac3.p_ctrl, g_dac3.p_cfg);
	
		R_DAC_Start(g_dac0.p_ctrl);
		R_DAC_Start(g_dac1.p_ctrl);
		R_DAC_Start(g_dac2.p_ctrl);
		R_DAC_Start(g_dac3.p_ctrl);
}

