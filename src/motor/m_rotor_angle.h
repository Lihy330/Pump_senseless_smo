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
	
	int16_t q15_delta_angle_Tangle;				// ���ٶȼ������ڵĽǶȱ仯��
	int16_t q15_delta_angle_Tangle_filtered;	// �Ƕȱ仯�����˲�ֵ
	uint8_t delta_angle_Tangle_cnt;				// �жϽ��ٶȼ������ڵļ�������
	
	int16_t q15_delta_angle_Tangle_sum;			// ���ٶȼ������ڹ����еĽǶȱ仯���ۼ�ֵ
	
	int16_t erps;	// ��ת�� 	תÿ��
	int16_t erps_filter1;
	int16_t erps_filter2;
	
	int16_t erpm_filter1;
	int16_t erpm_filter2;
	
	int16_t erpm;	// ��ת�� 	תÿ��
	int16_t rps;	// ��еת��	תÿ��
	int16_t rpm;	// ��еת��	תÿ��
	
	int16_t q15_delta_theta_e_filter_k;	// �Ƕȱ仯���˲�ϵ��
	
	
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