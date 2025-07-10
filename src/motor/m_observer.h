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
	
	// 第一次低通滤波输出
	int16_t q15_e_alpha_n_estimation;
	int16_t q15_e_beta_n_estimation;
	
	int16_t q15_e_alpha_n_1_estimation_final;
	int16_t q15_e_beta_n_1_estimation_final;
	
	// 第二次低通滤波输出（最终的低通滤波输出）
	int16_t q15_e_alpha_n_estimation_final;
	int16_t q15_e_beta_n_estimation_final;
	
	int16_t q15_z_alpha_n;
	int16_t q15_z_beta_n;
	
	int16_t q15_z_alpha_n_1;
	int16_t q15_z_beta_n_1;
	
	int16_t	q15_F;
	int16_t q15_G;
	
	int16_t q15_z_max;	// 滑膜观测器的边界层
	int16_t q15_k_gain;	// 滑膜观测器的增益K
	
	int16_t q15_klpf;	// 一阶数字低通滤波器的自适应滤波系数
	
}m_obs_unit_t;

extern m_obs_unit_t m_obs_unit;



#ifdef _cplusplus
}
#endif

#endif