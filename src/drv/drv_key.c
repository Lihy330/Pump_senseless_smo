#include <stdio.h>
#include "drv_key.h"
#include "drv_systick.h"

#define FILTER_KEEP_TIME   (10)

key_sparam_t key_st_sp_sparam;
key_sparam_t key_cw_ccw_sparam;

void drv_key_init(void)
{

}

void drv_key_scan()
{
	// START_STOP_KEY
	// Obtain the status of pressing or lifting
	bsp_io_level_t st_sp_key_value;
	R_IOPORT_PinRead(g_ioport.p_ctrl, START_STOP_KEY_PORT, &st_sp_key_value);
	if (st_sp_key_value == (bsp_io_level_t)KEY_DOWN_VALUE && key_st_sp_sparam.trigger == 0)
	{
		key_st_sp_sparam.trigger = 1;
		key_st_sp_sparam.down_time = drv_systick_time_get();
	}
	if (key_st_sp_sparam.trigger == 1)
	{
		// waiting for 10ms
		if (drv_systick_time_get() - key_st_sp_sparam.down_time > FILTER_KEEP_TIME)
		{
			R_IOPORT_PinRead(g_ioport.p_ctrl, START_STOP_KEY_PORT, &st_sp_key_value);
			if (st_sp_key_value == (bsp_io_level_t)KEY_DOWN_VALUE && key_st_sp_sparam.trigger == 1)
			{
				key_st_sp_sparam.trigger = 2;
				printf("The START_STOP_KEY has been pressed.");
			}
		}
	}
	else if (st_sp_key_value == (bsp_io_level_t)KEY_UP_VALUE && key_st_sp_sparam.trigger == 2)
	{
		key_st_sp_sparam.trigger = 0;
	}
	
	// START_STOP_KEY
	// Obtain the status of pressing or lifting
	bsp_io_level_t cw_ccw_key_value;
	R_IOPORT_PinRead(g_ioport.p_ctrl, CW_CCW_KEY_PORT, &cw_ccw_key_value);
	if (cw_ccw_key_value == (bsp_io_level_t)KEY_DOWN_VALUE && key_cw_ccw_sparam.trigger == 0)
	{
		key_cw_ccw_sparam.trigger = 1;
		key_cw_ccw_sparam.down_time = drv_systick_time_get();
	}
	if (key_cw_ccw_sparam.trigger == 1)
	{
		// waiting for 10ms
		if (drv_systick_time_get() - key_cw_ccw_sparam.down_time > FILTER_KEEP_TIME)
		{
			R_IOPORT_PinRead(g_ioport.p_ctrl, CW_CCW_KEY_PORT, &cw_ccw_key_value);
			if (cw_ccw_key_value == (bsp_io_level_t)KEY_DOWN_VALUE && key_cw_ccw_sparam.trigger == 1)
			{
				key_cw_ccw_sparam.trigger = 2;
				printf("The CW_CCW_KEY has been pressed.");
			}
		}
	}
	else if (cw_ccw_key_value == (bsp_io_level_t)KEY_UP_VALUE && key_cw_ccw_sparam.trigger == 2)
	{
		key_cw_ccw_sparam.trigger = 0;
	}
}
