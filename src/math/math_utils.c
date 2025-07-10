#include <stdio.h>
#include "math_utils.h"
#include "../motor/m_parameter.h"
#include "arm_math.h"



/*
	Yn = K * (Xn - Yn-1) + Yn-1
	Yn = K * Xn + (1 - K) * Yn-1
	E_n = K * Z_n + (1 - K) * E_n_1
	K ������Ӧ�˲�ϵ��
	�������ܣ�����һ�׵�ͨ�˲������㻬Ĥ���Ƶõ����������˲��õ��ķ��綯�ƹ���ֵ
*/
int16_t m_obs_digital_LPF(int16_t q15_klpf, int16_t q15_x, int16_t q15_y)
{
	
	int16_t	q15_y_res = (int16_t)(((int32_t)(q15_klpf * q15_x) + (int32_t)((32767 - q15_klpf) * q15_y)) >> 15);
	
	return q15_y_res;
}

/*
	�������ܣ�E��/E�� �����к������㣬����ֵΪq15��ʽ�ĽǶ�ֵ����Χ��-180~180��������Ƕȹ�һ���Ļ�׼ֵ��180��Ҳ���ǦУ�ֵ�ĺ���Ϊx����Ȧ
			  ���˻������ǣ������һ����Ϊ0.5����ô��Ӧ�ľ���0.5����Ȧ��Ҳ����90��
*/
int16_t	m_arctan_calculate(int16_t q15_e_alpha, int16_t q15_e_beta)
{
	float result_rad_f;
	float x_f;
	float y_f;
	int16_t arctan_res;
	
	// ������
	if (q15_e_alpha == 0 || q15_e_beta == 0)
	{
		return 0;
	}
	
	x_f = (float)q15_e_alpha / 32768.0f;
	y_f = (float)q15_e_beta / 32768.0f;
	
	
	arm_atan2_f32(y_f, x_f, &result_rad_f);
	
	// �޷�
	if (result_rad_f < -M_PI)
	{
		result_rad_f = -M_PI;
	}
	if (result_rad_f > M_PI)
	{
		result_rad_f = M_PI;
	}
	
	// ���Ƕȹ�һ��Ϊx����Ȧ(��׼Ϊ��)��Ȼ��ת����Q15����
	arctan_res = (int16_t)(result_rad_f / M_PI * 32767.0f);
	
	return arctan_res;
}

