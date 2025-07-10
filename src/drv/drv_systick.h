#ifndef __DRV_SYSTICK_H__
#define __DRV_SYSTICK_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

void drv_systick_init(void);
uint32_t drv_systick_time_get(void);
void drv_delay_ms(uint32_t time);

#ifdef __cplusplus
}
#endif


#endif
