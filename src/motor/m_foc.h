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
	uint16_t q16_theta_e_r_sub_angle; // ��λ����֮��Ħ�e���r֮��ĽǶȲ�
	int16_t q15_theta_e_r_sub_angle; // ��λ����֮��Ħ�e���r֮��ĽǶȲ�
	uint8_t open_to_close_cnt;		  // ��e���r֮��ĽǶȲ���60~120֮���������ֵĴ���
}m_drag_rotor_angle_t;

typedef struct
{
	uint16_t Us;					// Usģ��
	
	int16_t U_alpha;				// alpha���ѹֵ
	int16_t U_beta;					// beta���ѹֵ
	
	int16_t U_alpha_last;			// ��һ���ڵ�alpha���ѹֵ
	int16_t U_beta_last;			// ��һ���ڵ�beta���ѹֵ

	uint16_t rotor_angle;						// ת��λ�ý�
	int16_t rotor_angle_sin_value_q15;			// ת��λ�ýǵ�����ֵ
	int16_t rotor_angle_cos_value_q15;			// ת��λ�ýǵ�����ֵ
	
	uint16_t Us_angle;
	
	uint16_t theta_c;				// ��ǰ��
	
	int16_t theta_e_compensated;				// ��λ������ķ��綯�������λ�üн�
	
	uint16_t q16_spd_set_value;		// �ٶȻ�Ŀ��ת���趨ֵ
	uint16_t q16_spd_target_value;	// �ٶȻ�Ŀ��ת��ֵ
	
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
