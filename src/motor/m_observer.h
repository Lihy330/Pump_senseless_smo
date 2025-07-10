#ifndef __M_OBSERVER_H__
#define __M_OBSERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef struct
{
	int16_t q15_i_alpha_n_1_estimation;
	int16_t q15_i_beta_n_1_estimation;
	
	int16_t q15_i_alpha_n_estimation;
	int16_t q15_i_beta_n_estimation;
	
	int16_t q15_i_alpha_n;
	int16_t q15_i_beta_n;
	
	int16_t q15_u_alpha_n_1;
	int16_t q15_u_beta_n_1;
	
	int16_t q15_e_alpha_n_1_estimation;
	int16_t q15_e_beta_n_1_estimation;
	
	// ��һ�ε�ͨ�˲����
	int16_t q15_e_alpha_n_estimation;
	int16_t q15_e_beta_n_estimation;
	
	int16_t q15_e_alpha_n_1_estimation_final;
	int16_t q15_e_beta_n_1_estimation_final;
	
	// �ڶ��ε�ͨ�˲���������յĵ�ͨ�˲������
	int16_t q15_e_alpha_n_estimation_final;
	int16_t q15_e_beta_n_estimation_final;
	
	int16_t q15_z_alpha_n;
	int16_t q15_z_beta_n;
	
	int16_t q15_z_alpha_n_1;
	int16_t q15_z_beta_n_1;
	
	int16_t	q15_F;
	int16_t q15_G;
	
	int16_t q15_z_max;	// ��Ĥ�۲����ı߽��
	int16_t q15_k_gain;	// ��Ĥ�۲���������K
	
	int16_t q15_klpf;	// һ�����ֵ�ͨ�˲���������Ӧ�˲�ϵ��
	
}m_obs_unit_t;

extern m_obs_unit_t m_obs_unit;



#ifdef _cplusplus
}
#endif

#endif