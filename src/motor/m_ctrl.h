#ifndef __M_CTRL_H__
#define __M_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef enum
{
	MOTOR_STOP = 0,
	MOTOR_START,
	MOTOR_BOOST_CHARGING,
	MOTOR_ROTOR_PREPOS_START1,
	MOTOR_ROTOR_PREPOS_END1,
	MOTOR_ROTOR_PREPOS_START2,
	MOTOR_ROTOR_PREPOS_END2,
	MOTOR_DRAG,				// 电机强拖状态
	MOTOR_EXECUTE,
	MOTOR_TEST,
}m_motor_execute_state_machine_e;


typedef struct
{
	m_motor_execute_state_machine_e m_motor_execute_state_machine;
	uint8_t direction;
	volatile bool current_error_tick_flag;		// 该标志位用来判断是否已经完成了电流静态误差的采样
	volatile uint16_t speed;					// 电机的转速
	volatile uint16_t last_speed;				// 上一次执行速度环时的速度大小
	volatile bool speed_update_flag;			// 转速是否更新的标志
	volatile bool speed_cal_stable_flag;		// 转速计算稳定的标志
	volatile int16_t iq_start_value;			// q轴启动电流
	volatile uint8_t speed_cal_stable_cnt;		// 转速稳定的计数变量
	
}m_motor_ctrl_t;

extern m_motor_ctrl_t m_motor_ctrl;


void motor_start(void);
void motor_stop(void);
void motor_boost_charging(void);
void motor_execute(void);


#ifdef _cplusplus
}
#endif

#endif