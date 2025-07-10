#ifndef __M_TICK_H__
#define __M_TICK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef struct
{
	uint8_t spd_tick_cnt;
	uint8_t boost_charging_tick_cnt;
	uint16_t rotor_prepos1_tick_cnt;		// 第一次转子预定位的时间
	uint16_t rotor_prepos2_tick_cnt;		// 第二次转子预定位的时间
	uint16_t drag_tick_cnt;					// 开环强拖周期计数值
	uint16_t iq_calib_tick_cnt;				// 启动Iq电流逐渐衰减的周期计数变量
	uint16_t spd_loop_tick_cnt;				// 速度环执行周期
	uint16_t current_static_error_tick_cnt;
	uint16_t test_current_tick_cnt;			// 这个变量用来每隔50ms，打印一次当前采集到的三相电流值，调试时使用
	uint16_t speed_set_value_tick_cnt;		// 这个变量用来斜坡加速设置速度目标值的更新时间	200ms
	
}m_tick_unit_t;

extern m_tick_unit_t m_tick_unit;

void tick_execute(void);


#ifdef _cplusplus
}
#endif

#endif