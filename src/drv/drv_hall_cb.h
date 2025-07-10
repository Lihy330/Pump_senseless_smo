#ifndef __DRV_HALL_CB_H__
#define __DRV_HALL_CB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef struct
{
	uint32_t u_capture_value;
	uint32_t v_capture_value;
	uint32_t w_capture_value;
	uint32_t u_last_value;
	uint32_t v_last_value;
	uint32_t w_last_value;
	
	uint32_t u_th1_val;
	uint32_t u_th2_val;
	
	uint32_t v_th1_val;
	uint32_t v_th2_val;
	
	uint32_t w_th1_val;
	uint32_t w_th2_val;
	
	bool u_sign;
	bool u_trigger_sign;
	bool v_sign;
	bool v_trigger_sign;
	bool w_sign;
	bool w_trigger_sign;
	void (*hall_uvw_sign_clear_func)(void);
	void (*hall_reset_func)(void);
}drv_hall_capture_t;

extern drv_hall_capture_t drv_hall_capture;
	
#ifdef _cplusplus
}
#endif

#endif