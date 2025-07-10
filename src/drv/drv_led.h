#ifndef __DRV_LED_H__
#define __DRV_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

#define LED_ON 0
#define LED_OFF 1


typedef enum
{
	LED1 = 0,
	LED2,
	LED3,
	LED4
}led_num_e;

void drv_led_init(void);
void drv_led_ctrl(led_num_e num, uint8_t status);

#ifdef _cplusplus
}
#endif

#endif