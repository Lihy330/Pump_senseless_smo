#include <stdio.h>
#include "math_utils.h"
#include "../motor/m_parameter.h"
#include "arm_math.h"



/*
	Yn = K * (Xn - Yn-1) + Yn-1
	Yn = K * Xn + (1 - K) * Yn-1
	E_n = K * Z_n + (1 - K) * E_n_1
	K 是自适应滤波系数
	函数功能：数字一阶低通滤波，计算滑膜控制得到开关量后滤波得到的反电动势估计值
*/
int16_t m_obs_digital_LPF(int16_t q15_klpf, int16_t q15_x, int16_t q15_y)
{
	
	int16_t	q15_y_res = (int16_t)(((int32_t)(q15_klpf * q15_x) + (int32_t)((32767 - q15_klpf) * q15_y)) >> 15);
	
	return q15_y_res;
}

/*
	函数功能：Eα/Eβ 反正切函数计算，返回值为q15格式的角度值（范围是-180~180），这个角度归一化的基准值是180°也就是π，值的含义为x个半圈
			  用人话讲就是，比如归一化后为0.5，那么对应的就是0.5个半圈，也就是90°
*/
int16_t	m_arctan_calculate(int16_t q15_e_alpha, int16_t q15_e_beta)
{
	float result_rad_f;
	float x_f;
	float y_f;
	int16_t arctan_res;
	
	// 输入检查
	if (q15_e_alpha == 0 || q15_e_beta == 0)
	{
		return 0;
	}
	
	x_f = (float)q15_e_alpha / 32768.0f;
	y_f = (float)q15_e_beta / 32768.0f;
	
	
	arm_atan2_f32(y_f, x_f, &result_rad_f);
	
	// 限幅
	if (result_rad_f < -M_PI)
	{
		result_rad_f = -M_PI;
	}
	if (result_rad_f > M_PI)
	{
		result_rad_f = M_PI;
	}
	
	// 将角度归一化为x个半圈(基准为π)，然后转换成Q15数据
	arctan_res = (int16_t)(result_rad_f / M_PI * 32767.0f);
	
	return arctan_res;
}

