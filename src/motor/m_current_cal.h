#ifndef __M_CURRENT_CAL_H__
#define __M_CURRENT_CAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef struct
{
	// 三相Q15电流值
	int16_t q15_ia;
	int16_t q15_ib;
	int16_t q15_ic;
	
	// 实轴与虚轴的电流值
	int16_t q15_i_alpha;
	int16_t q15_i_beta;
	
	// d轴与q轴分量
	int16_t q15_id;
	int16_t q15_iq;
	
	// PI控制器的Ud和Uq的输出
	int16_t q15_Ud;
	int16_t q15_Uq;
	
	// Us最大模长
	int16_t Us_max;
	
}m_current_cal_unit_t;

extern m_current_cal_unit_t m_current_cal_unit;


void current_static_error_cal(void);
void test_current_func(void);
void calculate_Us_thetaC_func(void);
void chenggong_us_theta_c_calculate(void);
void clark_transform(void);
void park_transform(uint16_t theta);
void get_tripple_phase_current(void);

#ifdef _cplusplus
}
#endif

#endif