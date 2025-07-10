#include <stdio.h>
#include "math_trigonometric.h"
#include "../motor/m_parameter.h"


static const int16_t sin_func_value_table[128] = {0, 1608, 3212, 4808, 6393, 7962, 9512, 11039, 12540, 14010, 15447, 16846, 18205, \
		19520, 20788, 22006, 23170, 24279, 25330, 26320, 27246, 28106, 28899, 29622, 30274, 30853, 31357, 31786, 32138, 32413, 32610, \
		32729, 32767, 32729, 32610, 32413, 32138, 31786, 31357, 30853, 30274, 29622, 28899, 28106, 27246, 26320, 25330, 24279, 23170, \
		22006, 20788, 19520, 18205, 16846, 15447, 14010, 12540, 11039, 9512, 7962, 6393, 4808, 3212, 1608, 0, -1608, -3212, -4808, -6393, \
		-7962, -9512, -11039, -12540, -14010, -15447, -16846, -18205, -19520, -20788, -22006, -23170, -24279, -25330, -26320, -27246, -28106, \
		-28899, -29622, -30274, -30853, -31357, -31786, -32138, -32413, -32610, -32729, -32767, -32729, -32610, -32413, -32138, -31786, -31357, \
		-30853, -30274, -29622, -28899, -28106, -27246, -26320, -25330, -24279, -23170, -22006, -20788, -19520, -18205, -16846, -15447, -14010, \
		-12540, -11039, -9512, -7962, -6393, -4808, -3212, -1608
};

math_unit_t math_unit;

// y = y1 + k * ��x
int16_t linear_interpolation(uint16_t x, uint16_t x1, uint16_t x2, int16_t y1, int16_t y2)
{
	return y1 + (int16_t)(((y2 - y1) * (int16_t)(x - x1)) / (int16_t)(x2 - x1));
}

// ����thetaΪQ16��ʽ����
union_s32 sin_cos_calculate(uint16_t theta)
{
	union_s32 sin_cos_res;
	// ���㵱ǰ�Ƕ����ڵ�����λ��
	uint16_t quotient = theta / THETA_Q16_INTERVAL;				// ��
	uint16_t remainder = theta % THETA_Q16_INTERVAL;			// ����
	
	// ����Ϊ��˵����ǰ�Ƕȴ��������ұ��У�ֱ�Ӳ���ɻ�ȡ���Ƕ�ֵ
	if (!remainder)
	{
		sin_cos_res.words.high = sin_func_value_table[quotient];
		// ���ܻ�Խ�磬������Ҫ�����ұ�Ĵ�Сȡ��
		sin_cos_res.words.low = sin_func_value_table[(quotient + COS_INDEX_OFFSET) % SIN_TABLE_SIZE];
	}
	else	// ������Ϊ�����Ҫ�������Բ�ֵ
	{
		// ��ȡ���Ҷ˵�ĽǶ�ֵ���������ԽǶȵļ���õ���Ӧ��Q16�Ƕ�ֵ
		uint16_t x1 = quotient * THETA_Q16_INTERVAL;
		uint16_t x2 = ((quotient + 1) % SIN_TABLE_SIZE) * THETA_Q16_INTERVAL;
		int16_t sin_y1 = sin_func_value_table[quotient];
		int16_t sin_y2 = sin_func_value_table[(quotient + 1) % SIN_TABLE_SIZE];
		// ���Բ�ֵ��������ֵ
		sin_cos_res.words.high = linear_interpolation(theta, x1, x2, sin_y1, sin_y2);
		
		int16_t cos_y1 = sin_func_value_table[(quotient + COS_INDEX_OFFSET) % SIN_TABLE_SIZE];
		int16_t cos_y2 = sin_func_value_table[(quotient + 1 + COS_INDEX_OFFSET) % SIN_TABLE_SIZE];
		// ���Բ�ֵ��������ֵ
		sin_cos_res.words.low = linear_interpolation(theta, x1, x2, cos_y1, cos_y2);
	}
//	printf("%d\t", theta);
//	printf("%d\n", sin_cos_res.words.high);
	return sin_cos_res;
}
