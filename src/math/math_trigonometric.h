#ifndef __MATH_TRIGONOMETRIC_H__
#define __MATH_TRIGONOMETRIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"
#include "../motor/m_parameter.h"

// 正弦表的大小，也就是将0~360°分成了多少个角度值
#define SIN_TABLE_SIZE						128
#define THETA_Q16_INTERVAL			(uint16_t)(65536.0f / (float)SIN_TABLE_SIZE)
	
// 余弦与正弦的索引偏移量，余弦与正弦相位差是 π/2，也就是四分之一圆周，所以是 SIN_TABLE_SIZE / 4
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
