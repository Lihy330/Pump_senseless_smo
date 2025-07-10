#ifndef __M_FOC_H__
#define __M_FOC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef struct
{
	uint16_t q16_drag_acc_value;
	uint16_t q16_drag_acc_value_last;
	uint16_t q16_drag_w_value;
	uint16_t q16_drag_w_value_last;
	uint16_t q16_theta_e_r_sub_angle; // 相位补偿之后的θe与θr之间的角度差
	int16_t q15_theta_e_r_sub_angle; // 相位补偿之后的θe与θr之间的角度差
	uint8_t open_to_close_cnt;		  // θe与θr之间的角度差在60~120之间连续出现的次数
}m_drag_rotor_angle_t;

typedef struct
{
	uint16_t Us;					// Us模长
	
	int16_t U_alpha;				// alpha轴电压值
	int16_t U_beta;					// beta轴电压值
	
	int16_t U_alpha_last;			// 上一周期的alpha轴电压值
	int16_t U_beta_last;			// 上一周期的beta轴电压值

	uint16_t rotor_angle;						// 转子位置角
	int16_t rotor_angle_sin_value_q15;			// 转子位置角的正弦值
	int16_t rotor_angle_cos_value_q15;			// 转子位置角的余弦值
	
	uint16_t Us_angle;
	
	uint16_t theta_c;				// 超前角
	
	int16_t theta_e_compensated;				// 相位补偿后的反电动势与零度位置夹角
	
	uint16_t q16_spd_set_value;		// 速度环目标转速设定值
	uint16_t q16_spd_target_value;	// 速度环目标转速值
	
	m_drag_rotor_angle_t m_drag_rotor_angle;
	
}m_foc_unit_t;

extern m_foc_unit_t m_foc_unit;

void m_calculate_Us(void);

void m_foc_algorithm_execute(void);

void current_pid_execute(void);


#ifdef _cplusplus
}
#endif

#endif
