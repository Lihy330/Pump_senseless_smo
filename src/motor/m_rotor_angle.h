#ifndef __M_ROTOR_ANGLE_H__
#define __M_ROTOR_ANGLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef struct
{
	int16_t q15_theta_e_uncompensated;
	int16_t q15_theta_e_uncompensated_last;
	
	int16_t q15_theta_e_compensated;
	
	int16_t q15_delta_angle_Tangle;				// 角速度计算周期的角度变化量
	int16_t q15_delta_angle_Tangle_filtered;	// 角度变化量的滤波值
	uint8_t delta_angle_Tangle_cnt;				// 判断角速度计算周期的计数变量
	
	int16_t q15_delta_angle_Tangle_sum;			// 角速度计算周期过程中的角度变化量累加值
	
	int16_t erps;	// 电转速 	转每秒
	int16_t erps_filter1;
	int16_t erps_filter2;
	
	int16_t erpm_filter1;
	int16_t erpm_filter2;
	
	int16_t erpm;	// 电转速 	转每分
	int16_t rps;	// 机械转速	转每秒
	int16_t rpm;	// 机械转速	转每分
	
	int16_t q15_delta_theta_e_filter_k;	// 角度变化量滤波系数
	
	
}m_obs_angle_unit_t;

extern m_obs_angle_unit_t m_obs_angle_unit;


void m_rotor_angle_init(void);
uint16_t m_rotor_angle_calculate(void);
	
void m_sensorless_theta_e_calculate();

void m_sensorless_rpm_calculate();

int16_t m_sensorless_theta_e_execute(void);

#ifdef _cplusplus
}
#endif

#endif