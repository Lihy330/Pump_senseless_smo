#ifndef __MATH_TRIGONOMETRIC_H__
#define __MATH_TRIGONOMETRIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"
#include "../motor/m_parameter.h"

// ���ұ�Ĵ�С��Ҳ���ǽ�0~360��ֳ��˶��ٸ��Ƕ�ֵ
#define SIN_TABLE_SIZE						128
#define THETA_Q16_INTERVAL			(uint16_t)(65536.0f / (float)SIN_TABLE_SIZE)
	
// ���������ҵ�����ƫ������������������λ���� ��/2��Ҳ�����ķ�֮һԲ�ܣ������� SIN_TABLE_SIZE / 4
#define COS_INDEX_OFFSET			(uint16_t)((float)SIN_TABLE_SIZE / 4.0f)   

typedef struct
{
	int16_t sin_value_q15;
	int16_t cos_value_q15;
}math_unit_t;

extern math_unit_t math_unit;

union_s32 sin_cos_calculate(uint16_t theta);

#ifdef _cplusplus
}
#endif

#endif
