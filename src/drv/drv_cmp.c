#include <stdio.h>
#include "drv_cmp.h"

void drv_cmp_init()
{
		R_ACMPHS_Open(g_comparator0.p_ctrl, g_comparator0.p_cfg);
		
		R_ACMPHS_OutputEnable(g_comparator0.p_ctrl);
}

