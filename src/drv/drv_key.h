#ifndef __DRV_KEY_H__
#define __DRV_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

#define KEY_DOWN_VALUE 0
#define KEY_UP_VALUE 1

#define START_STOP_KEY_PORT 	BSP_IO_PORT_12_PIN_07
#define CW_CCW_KEY_PORT 			BSP_IO_PORT_12_PIN_06

typedef enum
{
	START_STOP_KEY = 0,
	CW_CCW_KEY,
	KEY_MAX
}key_num_e;

typedef struct
{
	uint32_t down_time; 	// time of key press
	uint8_t trigger;    	// status count
	uint32_t down_count;	// key pressed count
}key_sparam_t;

extern key_sparam_t key_st_sp_sparam;
extern key_sparam_t key_cw_ccw_sparam;

void drv_key_init(void);
void drv_key_scan(void);

#ifdef _cplusplus
}
#endif

#endif