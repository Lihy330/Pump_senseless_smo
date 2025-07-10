#include <stdio.h>
#include "drv_hall_cb.h"


static void hall_uvw_sign_clear()
{
	static uint8_t test_io_1_level = 0;
	test_io_1_level = !test_io_1_level;
	R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_12, test_io_1_level);
	drv_hall_capture.u_sign = false;
	drv_hall_capture.v_sign = false;
	drv_hall_capture.w_sign = false;
}

static void hall_reset()
{
	memset(&drv_hall_capture, 0, sizeof(drv_hall_capture));
}


drv_hall_capture_t drv_hall_capture = 
{
	.hall_uvw_sign_clear_func = hall_uvw_sign_clear,
	.hall_reset_func = hall_reset,
};


void g_timer4_callback(timer_callback_args_t* p_args)
{
	if (p_args->event == TIMER_EVENT_CAPTURE_B)
	{
		#if 0
		uint32_t current_capture_value = 0;
		uint32_t last_capture_value = drv_hall_capture.u_last_value;
		if (last_capture_value == 0)
		{
			drv_hall_capture.u_last_value = p_args->capture;
		}
		else
		{
			current_capture_value = p_args->capture;
			drv_hall_capture.u_last_value = current_capture_value;
			if (current_capture_value > last_capture_value)
			{
				drv_hall_capture.u_capture_value = current_capture_value - last_capture_value;
			}
			else
			{
				drv_hall_capture.u_capture_value = 0xFFFFFFFE - last_capture_value + current_capture_value;
			}
			drv_hall_capture.u_sign = true;
		}
		#else
		drv_hall_capture.u_capture_value = p_args->capture;
		drv_hall_capture.u_sign = true;
		#endif
		
	}
}


void g_timer5_callback(timer_callback_args_t* p_args)
{
	if (p_args->event == TIMER_EVENT_CAPTURE_A)
	{
		#if 0
		uint32_t current_capture_value = 0;
		uint32_t last_capture_value = drv_hall_capture.v_last_value;
		if (last_capture_value == 0)
		{
			drv_hall_capture.v_last_value = p_args->capture;
		}
		else
		{
			current_capture_value = p_args->capture;
			drv_hall_capture.v_last_value = current_capture_value;
			if (current_capture_value > last_capture_value)
			{
				drv_hall_capture.v_capture_value = current_capture_value - last_capture_value;
			}
			else
			{
				drv_hall_capture.v_capture_value = 0xFFFFFFFE - last_capture_value + current_capture_value;
			}
			drv_hall_capture.v_sign = true;
		}
		#else
		drv_hall_capture.v_capture_value = p_args->capture;
		drv_hall_capture.v_sign = true;
		#endif
	}
	else if (p_args->event == TIMER_EVENT_CAPTURE_B)
	{
		#if 0
		uint32_t current_capture_value = 0;
		uint32_t last_capture_value = drv_hall_capture.w_last_value;
		if (last_capture_value == 0)
		{
			drv_hall_capture.w_last_value = p_args->capture;
		}
		else
		{
			current_capture_value = p_args->capture;
			drv_hall_capture.w_last_value = current_capture_value;
			if (current_capture_value > last_capture_value)
			{
				drv_hall_capture.w_capture_value = current_capture_value - last_capture_value;
			}
			else
			{
				drv_hall_capture.w_capture_value = 0xFFFFFFFE - last_capture_value + current_capture_value;
			}
			drv_hall_capture.w_sign = true;
		}
		#else
		drv_hall_capture.w_capture_value = p_args->capture;
		drv_hall_capture.w_sign = true;
		#endif
	}
}



//void g_timer4_callback(timer_callback_args_t* p_args)
//{
//	if(p_args->event == TIMER_EVENT_CAPTURE_B)
//	{
//		if(drv_hall_capture.u_trigger_sign == false)
//		{
//			drv_hall_capture.u_th1_val = p_args->capture;
//			drv_hall_capture.u_trigger_sign = true;
//			/*2个延都捕获到了值:180°电角度时间值，周期1us*/
//			if(drv_hall_capture.u_th1_val != 0 && drv_hall_capture.u_th2_val != 0)
//			{
//				if(drv_hall_capture.u_th1_val >= drv_hall_capture.u_th2_val)
//				{
//					drv_hall_capture.u_capture_value = drv_hall_capture.u_th1_val - drv_hall_capture.u_th2_val;
//				}
//				else
//				{
//					drv_hall_capture.u_capture_value = 0xFFFFFFFF - drv_hall_capture.u_th2_val + drv_hall_capture.u_th1_val;
//				}
//				drv_hall_capture.u_capture_val = drv_hall_capture.u_capture_val >> 2;
//				drv_hall_capture.u_sign = true;
//			}
//		}
//		else if(drv_hall_capture.u_trigger_sign == true)
//		{
//			drv_hall_capture.u_th2_val = p_args->capture;
//			drv_hall_capture.u_trigger_sign = false;
//			
//			/*2个延都捕获到了值:180°电角度时间值，周期1us*/
//			if(drv_hall_capture.u_th1_val != 0 && drv_hall_capture.u_th2_val != 0)
//			{
//				if(drv_hall_capture.u_th2_val >= drv_hall_capture.u_th1_val)
//				{
//					drv_hall_capture.u_capture_value = drv_hall_capture.u_th2_val - drv_hall_capture.u_th1_val;
//				}
//				else
//				{
//					drv_hall_capture.u_capture_value = 0xFFFFFFFF - drv_hall_capture.u_th1_val + drv_hall_capture.u_th2_val;
//				}
//				drv_hall_capture.u_capture_val = drv_hall_capture.u_capture_val >> 2;
//				drv_hall_capture.u_sign = true;
//			}
//		}
//	}
//}


//void g_timer5_callback(timer_callback_args_t * p_args)
//{
//	/*hall v capture*/
//	if(p_args->event == TIMER_EVENT_CAPTURE_A)
//	{	
//		if(drv_hall_capture.v_trigger_sign == false)
//		{
//			drv_hall_capture.v_th1_val = p_args->capture;
//			drv_hall_capture.v_trigger_sign = true;
//			/*2个延都捕获到了值:180°电角度时间值，周期1us*/
//			if(drv_hall_capture.v_th1_val != 0 && drv_hall_capture.v_th2_val != 0)
//			{
//				if(drv_hall_capture.v_th1_val >= drv_hall_capture.v_th2_val)
//				{
//					drv_hall_capture.v_capture_value = drv_hall_capture.v_th1_val - drv_hall_capture.v_th2_val;
//				}
//				else
//				{
//					drv_hall_capture.v_capture_value = 0xFFFFFFFF - drv_hall_capture.v_th2_val + drv_hall_capture.v_th1_val;
//				}
//				drv_hall_capture.v_capture_val = drv_hall_capture.v_capture_val >> 2;
//				drv_hall_capture.v_sign = true;
//			}
//		}
//		else if(drv_hall_capture.v_trigger_sign == true)
//		{
//			drv_hall_capture.v_th2_val = p_args->capture;
//			drv_hall_capture.v_trigger_sign = false;
//			
//			/*2个延都捕获到了值:180°电角度时间值，周期1us*/
//			if(drv_hall_capture.v_th1_val != 0 && drv_hall_capture.v_th2_val != 0)
//			{
//				if(drv_hall_capture.v_th2_val >= drv_hall_capture.v_th1_val)
//				{
//					drv_hall_capture.v_capture_value = drv_hall_capture.v_th2_val - drv_hall_capture.v_th1_val;
//				}
//				else
//				{
//					drv_hall_capture.v_capture_value = 0xFFFFFFFF - drv_hall_capture.v_th1_val + drv_hall_capture.v_th2_val;
//				}
//				drv_hall_capture.v_capture_val = drv_hall_capture.v_capture_val >> 2;
//				drv_hall_capture.v_sign = true;
//			}
//		}
//	}
//	/*hall w capture*/
//	else if(p_args->event == TIMER_EVENT_CAPTURE_B)
//	{	
//		if(drv_hall_capture.w_trigger_sign == false)
//		{
//			drv_hall_capture.w_th1_val = p_args->capture;
//			drv_hall_capture.w_trigger_sign = true;
//			/*2个延都捕获到了值:180°电角度时间值，周期1us*/
//			if(drv_hall_capture.w_th1_val != 0 && drv_hall_capture.w_th2_val != 0)
//			{
//				if(drv_hall_capture.w_th1_val >= drv_hall_capture.w_th2_val)
//				{
//					drv_hall_capture.w_capture_value = drv_hall_capture.w_th1_val - drv_hall_capture.w_th2_val;
//				}
//				else
//				{
//					drv_hall_capture.w_capture_value = 0xFFFFFFFF - drv_hall_capture.w_th2_val + drv_hall_capture.w_th1_val;
//				}
//				drv_hall_capture.w_sign = true;
//				drv_hall_capture.w_capture_val = drv_hall_capture.w_capture_val >> 2;
//			}
//		}
//		else if(drv_hall_capture.w_trigger_sign == true)
//		{
//			drv_hall_capture.w_th2_val = p_args->capture;
//			drv_hall_capture.w_trigger_sign = false;
//			
//			/*2个延都捕获到了值:180°电角度时间值，周期1us*/
//			if(drv_hall_capture.w_th1_val != 0 && drv_hall_capture.w_th2_val != 0)
//			{
//				if(drv_hall_capture.w_th2_val >= drv_hall_capture.w_th1_val)
//				{
//					drv_hall_capture.w_capture_value = drv_hall_capture.w_th2_val - drv_hall_capture.w_th1_val;
//				}
//				else
//				{
//					drv_hall_capture.w_capture_value = 0xFFFFFFFF - drv_hall_capture.w_th1_val + drv_hall_capture.w_th2_val;
//				}
//				drv_hall_capture.w_capture_val = drv_hall_capture.w_capture_val >> 2;
//				drv_hall_capture.w_sign = true;
//			}
//		}
//	}
//}


