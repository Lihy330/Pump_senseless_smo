#ifndef __M_SVPWM_H__
#define __M_SVPWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef struct
{
	uint8_t sector;
	int16_t generic_x;	// x = sina; 
	int16_t generic_y;	// y = (1/2)sina - (sqrt(3)/2)cosa;
	int16_t generic_z;	// z = (1/2)sina + (sqrt(3)/2)cosa;
	uint16_t q16ta;
	uint16_t q16tb;
	uint16_t tcout;
	uint16_t tbout;
	uint16_t taout;
	uint16_t u_duty_setting_value;
	uint16_t v_duty_setting_value;
	uint16_t w_duty_setting_value;
}m_svpwm_unit_t;

extern m_svpwm_unit_t m_svpwm_unit;

void m_svpwm_generate(uint16_t Us, uint16_t theta);

#ifdef _cplusplus
}
#endif

#endif

