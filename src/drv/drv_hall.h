#ifndef __DRV_HALL_H__
#define __DRV_HALL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

#define HALL_U_IO_PORT 	BSP_IO_PORT_11_PIN_05
#define HALL_V_IO_PORT 	BSP_IO_PORT_11_PIN_06
#define HALL_W_IO_PORT 	BSP_IO_PORT_11_PIN_07


void drv_hall_init(void);

#ifdef _cplusplus
}
#endif

#endif