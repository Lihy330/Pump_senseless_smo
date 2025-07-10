#include <stdio.h>
#include "drv_adc.h"

void drv_adc_init()
{
	R_ADC_B_Open(&g_adc0_ctrl, &g_adc0_cfg);
	R_ADC_B_ScanCfg(&g_adc0_ctrl, &g_adc0_scan_cfg);
	R_ADC_B_ScanGroupStart(&g_adc0_ctrl, ADC_GROUP_MASK_0 | ADC_GROUP_MASK_1);
}

