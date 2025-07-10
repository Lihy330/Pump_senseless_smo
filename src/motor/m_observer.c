#include <stdio.h>
#include "m_observer.h"
#include "m_parameter.h"
#include "m_foc.h"
#include "m_current_cal.h"

m_obs_unit_t m_obs_unit;


/*
	I_��_n_estimation = F * I_��_n_1_estimation + G * U_��_n_1 - G * Z_��_n_1 - G * E_��_n_1_estimation
		F = 1 - (Ts / L) * R	������
		G = Ts / L				����Ϊ 1 / ��
	q15_i_n_1: ��һ���ڹ��Ƶĵ���ֵ
	q15_u_n_1: ��һ����alpha����beta��ĵ�ѹֵ
	q15_z_n_1: ��һ���ڻ�Ĥ�۲��������ֵ
	q15_e_n_1: ��һ���ڹ��Ƶķ��綯��ֵ
	
	�������ܣ����Ƶ�ǰ���ڵĵ���ֵ
*/
int16_t m_obs_current_estimate(int16_t q15_F, int16_t q15_G, int16_t q15_i_n_1_estimation, int16_t q15_u_n_1, int16_t q15_z_n_1, int16_t q15_e_n_1_estimation)
{
	int32_t temp = 0;
	temp = (int32_t)(q15_F * q15_i_n_1_estimation) + (int32_t)(q15_G * q15_u_n_1) - (int32_t)(q15_G * q15_z_n_1) - (int32_t)(q15_G * q15_e_n_1_estimation);
	
	return (int16_t)(temp >> 15);
}


/*
	q15_i_n_estimation: ��ǰ���ڵĵ�������ֵ
	q15_i_n: 			��ǰ���ڵĵ���ʵ��ֵ
	q15_z_max: 			��Ĥ�۲�����������ֵ
	q15_k_gain: 		��Ĥ�۲���������

	�������ܣ����㻬Ĥ�۲��������z
*/
int16_t m_obs_z_calculate(int16_t q15_i_n_estimation, int16_t q15_i_n, int16_t q15_z_max, int16_t q15_k_gain)
{
	int16_t obs_z_res;
	
	int16_t q15_i_error = q15_i_n_estimation - q15_i_n;
	
	if (q15_i_error > q15_z_max)
	{
		obs_z_res = q15_k_gain;
	}
	else if (q15_i_error < -q15_z_max)
	{
		obs_z_res = -q15_k_gain;
	}
	else
	{
		obs_z_res = (int16_t)((int32_t)(q15_k_gain * q15_i_error) / q15_z_max);
	}
	return obs_z_res;
}


/*
	��ʼ����Ĥ�۲�������ز���
*/
void m_obs_initial()
{
	// F = 1 - R * (Tpwm / L)
	m_obs_unit.q15_F = F_COEFF;
	// G = Tpwm / L
	m_obs_unit.q15_G = G_COEFF;
	m_obs_unit.q15_z_max = OBS_Z_MAX;
	m_obs_unit.q15_k_gain = OBS_K_GAIN;
}

void m_obs_execute()
{
	// 1. ��ȡ��ǰ���ڲ����ĵ��Ħ���beta��ĵ���ֵ
	m_obs_unit.q15_i_alpha_n = m_current_cal_unit.q15_i_alpha;
	m_obs_unit.q15_i_beta_n = m_current_cal_unit.q15_i_beta;
	
	// 2.��ȡ�ϸ�����Usʸ���ֽ�õ��Ħ������ĵ�ѹֵ
	m_obs_unit.q15_u_alpha_n_1 = m_foc_unit.U_alpha_last;
	m_obs_unit.q15_u_beta_n_1 = m_foc_unit.U_beta_last;
	
	// 3.���ڻ�Ĥ�۲������㵱ǰ���ڦ����������Ĺ���ֵ
	m_obs_unit.q15_i_alpha_n_estimation = m_obs_current_estimate(m_obs_unit.q15_F, m_obs_unit.q15_G, m_obs_unit.q15_i_alpha_n_1_estimation, m_obs_unit.q15_u_alpha_n_1, m_obs_unit.q15_z_alpha_n_1, m_obs_unit.q15_e_alpha_n_1_estimation);
	m_obs_unit.q15_i_beta_n_estimation = m_obs_current_estimate(m_obs_unit.q15_F, m_obs_unit.q15_G, m_obs_unit.q15_i_beta_n_1_estimation, m_obs_unit.q15_u_beta_n_1, m_obs_unit.q15_z_beta_n_1, m_obs_unit.q15_e_beta_n_1_estimation);

	// 4.���㵱ǰ���ڻ�Ĥ�۲��������z
	m_obs_unit.q15_z_alpha_n = m_obs_z_calculate(m_obs_unit.q15_i_alpha_n_estimation, m_obs_unit.q15_i_alpha_n, m_obs_unit.q15_z_max, m_obs_unit.q15_k_gain);
	m_obs_unit.q15_z_beta_n = m_obs_z_calculate(m_obs_unit.q15_i_beta_n_estimation, m_obs_unit.q15_i_beta_n, m_obs_unit.q15_z_max, m_obs_unit.q15_k_gain);
	
	// 5.����һ�����ֵ�ͨ�˲����õ����綯�ƵĹ���ֵ
	// ��һ�ε�ͨ�˲�
	m_obs_unit.q15_e_alpha_n_estimation = m_obs_digital_LPF(m_obs_unit.q15_klpf, m_obs_unit.q15_z_alpha_n, m_obs_unit.q15_e_alpha_n_1_estimation);
	m_obs_unit.q15_e_beta_n_estimation = m_obs_digital_LPF(m_obs_unit.q15_klpf, m_obs_unit.q15_z_beta_n, m_obs_unit.q15_e_beta_n_1_estimation);
	// �ڶ��ε�ͨ�˲�
	m_obs_unit.q15_e_alpha_n_estimation_final = m_obs_digital_LPF(m_obs_unit.q15_klpf, m_obs_unit.q15_e_alpha_n_estimation, m_obs_unit.q15_e_alpha_n_1_estimation_final);
	m_obs_unit.q15_e_beta_n_estimation_final = m_obs_digital_LPF(m_obs_unit.q15_klpf, m_obs_unit.q15_e_beta_n_estimation, m_obs_unit.q15_e_beta_n_1_estimation_final);
	
	
	// ����������һ��ֵ
	// ��������ֵ
	m_obs_unit.q15_i_alpha_n_1_estimation = m_obs_unit.q15_i_alpha_n_estimation;
	m_obs_unit.q15_i_beta_n_1_estimation = m_obs_unit.q15_i_beta_n_estimation;
	
	// ��Ĥ�۲���z
	m_obs_unit.q15_z_alpha_n_1 = m_obs_unit.q15_z_alpha_n;
	m_obs_unit.q15_z_beta_n_1 = m_obs_unit.q15_z_beta_n;
	
	// ��һ�ε�ͨ�˲����
	m_obs_unit.q15_e_alpha_n_1_estimation = m_obs_unit.q15_e_alpha_n_estimation;
	m_obs_unit.q15_e_beta_n_1_estimation = m_obs_unit.q15_e_beta_n_estimation;
	// �ڶ��ε�ͨ�˲����
	m_obs_unit.q15_e_alpha_n_1_estimation_final = m_obs_unit.q15_e_alpha_n_estimation_final;
	m_obs_unit.q15_e_beta_n_1_estimation_final = m_obs_unit.q15_e_beta_n_estimation_final;
	
}



