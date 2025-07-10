#include <stdio.h>
#include "m_observer.h"
#include "m_parameter.h"
#include "m_foc.h"
#include "m_current_cal.h"

m_obs_unit_t m_obs_unit;


/*
	I_α_n_estimation = F * I_α_n_1_estimation + G * U_α_n_1 - G * Z_α_n_1 - G * E_α_n_1_estimation
		F = 1 - (Ts / L) * R	无量纲
		G = Ts / L				量纲为 1 / Ω
	q15_i_n_1: 上一周期估计的电流值
	q15_u_n_1: 上一周期alpha或者beta相的电压值
	q15_z_n_1: 上一周期滑膜观测器的输出值
	q15_e_n_1: 上一周期估计的反电动势值
	
	函数功能：估计当前周期的电流值
*/
int16_t m_obs_current_estimate(int16_t q15_F, int16_t q15_G, int16_t q15_i_n_1_estimation, int16_t q15_u_n_1, int16_t q15_z_n_1, int16_t q15_e_n_1_estimation)
{
	int32_t temp = 0;
	temp = (int32_t)(q15_F * q15_i_n_1_estimation) + (int32_t)(q15_G * q15_u_n_1) - (int32_t)(q15_G * q15_z_n_1) - (int32_t)(q15_G * q15_e_n_1_estimation);
	
	return (int16_t)(temp >> 15);
}


/*
	q15_i_n_estimation: 当前周期的电流估计值
	q15_i_n: 			当前周期的电流实际值
	q15_z_max: 			滑膜观测器的最大误差值
	q15_k_gain: 		滑膜观测器的增益

	函数功能：计算滑膜观测器的输出z
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
	初始化滑膜观测器的相关参数
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
	// 1. 获取当前周期采样的到的α与beta轴的电流值
	m_obs_unit.q15_i_alpha_n = m_current_cal_unit.q15_i_alpha;
	m_obs_unit.q15_i_beta_n = m_current_cal_unit.q15_i_beta;
	
	// 2.获取上个周期Us矢量分解得到的α与β轴的电压值
	m_obs_unit.q15_u_alpha_n_1 = m_foc_unit.U_alpha_last;
	m_obs_unit.q15_u_beta_n_1 = m_foc_unit.U_beta_last;
	
	// 3.基于滑膜观测器计算当前周期α与β轴电流的估计值
	m_obs_unit.q15_i_alpha_n_estimation = m_obs_current_estimate(m_obs_unit.q15_F, m_obs_unit.q15_G, m_obs_unit.q15_i_alpha_n_1_estimation, m_obs_unit.q15_u_alpha_n_1, m_obs_unit.q15_z_alpha_n_1, m_obs_unit.q15_e_alpha_n_1_estimation);
	m_obs_unit.q15_i_beta_n_estimation = m_obs_current_estimate(m_obs_unit.q15_F, m_obs_unit.q15_G, m_obs_unit.q15_i_beta_n_1_estimation, m_obs_unit.q15_u_beta_n_1, m_obs_unit.q15_z_beta_n_1, m_obs_unit.q15_e_beta_n_1_estimation);

	// 4.计算当前周期滑膜观测器的输出z
	m_obs_unit.q15_z_alpha_n = m_obs_z_calculate(m_obs_unit.q15_i_alpha_n_estimation, m_obs_unit.q15_i_alpha_n, m_obs_unit.q15_z_max, m_obs_unit.q15_k_gain);
	m_obs_unit.q15_z_beta_n = m_obs_z_calculate(m_obs_unit.q15_i_beta_n_estimation, m_obs_unit.q15_i_beta_n, m_obs_unit.q15_z_max, m_obs_unit.q15_k_gain);
	
	// 5.基于一阶数字低通滤波器得到反电动势的估计值
	// 第一次低通滤波
	m_obs_unit.q15_e_alpha_n_estimation = m_obs_digital_LPF(m_obs_unit.q15_klpf, m_obs_unit.q15_z_alpha_n, m_obs_unit.q15_e_alpha_n_1_estimation);
	m_obs_unit.q15_e_beta_n_estimation = m_obs_digital_LPF(m_obs_unit.q15_klpf, m_obs_unit.q15_z_beta_n, m_obs_unit.q15_e_beta_n_1_estimation);
	// 第二次低通滤波
	m_obs_unit.q15_e_alpha_n_estimation_final = m_obs_digital_LPF(m_obs_unit.q15_klpf, m_obs_unit.q15_e_alpha_n_estimation, m_obs_unit.q15_e_alpha_n_1_estimation_final);
	m_obs_unit.q15_e_beta_n_estimation_final = m_obs_digital_LPF(m_obs_unit.q15_klpf, m_obs_unit.q15_e_beta_n_estimation, m_obs_unit.q15_e_beta_n_1_estimation_final);
	
	
	// 更新所有上一次值
	// 电流估计值
	m_obs_unit.q15_i_alpha_n_1_estimation = m_obs_unit.q15_i_alpha_n_estimation;
	m_obs_unit.q15_i_beta_n_1_estimation = m_obs_unit.q15_i_beta_n_estimation;
	
	// 滑膜观测器z
	m_obs_unit.q15_z_alpha_n_1 = m_obs_unit.q15_z_alpha_n;
	m_obs_unit.q15_z_beta_n_1 = m_obs_unit.q15_z_beta_n;
	
	// 第一次低通滤波输出
	m_obs_unit.q15_e_alpha_n_1_estimation = m_obs_unit.q15_e_alpha_n_estimation;
	m_obs_unit.q15_e_beta_n_1_estimation = m_obs_unit.q15_e_beta_n_estimation;
	// 第二次低通滤波输出
	m_obs_unit.q15_e_alpha_n_1_estimation_final = m_obs_unit.q15_e_alpha_n_estimation_final;
	m_obs_unit.q15_e_beta_n_1_estimation_final = m_obs_unit.q15_e_beta_n_estimation_final;
	
}



