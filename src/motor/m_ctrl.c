#include <stdio.h>
#include "m_ctrl.h"
#include "m_foc.h"
#include "m_parameter.h"

m_motor_ctrl_t m_motor_ctrl;


void motor_boost_charging()
{
	m_motor_ctrl.m_motor_execute_state_machine = MOTOR_BOOST_CHARGING;
	
	three_phase_duty_cycle_t three_phase_duty_cycle = {0};
	
	// 三相上管关闭，下管开启，自举电容预充电
	three_phase_duty_cycle.duty[0] = GPT_RELOAD_VALUE;
	three_phase_duty_cycle.duty[1] = GPT_RELOAD_VALUE;
	three_phase_duty_cycle.duty[2] = GPT_RELOAD_VALUE;
	
	R_GPT_THREE_PHASE_DutyCycleSet(g_three_phase0.p_ctrl, &three_phase_duty_cycle);
//	printf("Motor charging..............................\r\n");
}


void motor_start()
{
	m_motor_ctrl.m_motor_execute_state_machine = MOTOR_START;
	
	three_phase_duty_cycle_t three_phase_duty_cycle = {0};
	
	three_phase_duty_cycle.duty[0] = GPT_RELOAD_VALUE;
	three_phase_duty_cycle.duty[1] = GPT_RELOAD_VALUE;
	three_phase_duty_cycle.duty[2] = GPT_RELOAD_VALUE;
	
	R_GPT_THREE_PHASE_DutyCycleSet(g_three_phase0.p_ctrl, &three_phase_duty_cycle);
//	printf("Motor start..............................\r\n");
}

void motor_stop()
{
	m_motor_ctrl.m_motor_execute_state_machine = MOTOR_STOP;
	
	three_phase_duty_cycle_t three_phase_duty_cycle = {0};
	
	// 采取折中的办法，没有办法使得GPT定时器Disable，因为ADC采集需要GPT定时器计数，所以只能通过将上管占空比设置为0来停止输出
	// 安全起见，一旦电机停机，就将电源断掉即可
	three_phase_duty_cycle.duty[0] = GPT_RELOAD_VALUE;
	three_phase_duty_cycle.duty[1] = GPT_RELOAD_VALUE;
	three_phase_duty_cycle.duty[2] = GPT_RELOAD_VALUE;
	
	R_GPT_THREE_PHASE_DutyCycleSet(g_three_phase0.p_ctrl, &three_phase_duty_cycle);
}


void motor_execute_ctrl()
{
	if(m_motor_ctrl.m_motor_execute_state_machine == MOTOR_STOP)
	{
		if (m_foc_unit.q16_spd_set_value > 0)
		{
			motor_start();
		}
	}
	else if(m_motor_ctrl.m_motor_execute_state_machine != MOTOR_STOP)
	{
		if (m_foc_unit.q16_spd_set_value == 0)
		{
			motor_stop();
		}
	}
}

// 初始化与电机相关的所有参数
void motor_init()
{
	m_motor_ctrl.speed = 0;
	m_motor_ctrl.last_speed = 0;
	m_motor_ctrl.speed_cal_stable_flag = false;
	m_motor_ctrl.speed_update_flag = false;
	m_motor_ctrl.speed_cal_stable_cnt = 0;
	
	// 初始化q轴起始电流
	switch(m_motor_ctrl.direction)
	{
		case CCW:
			m_motor_ctrl.iq_start_value = IQ_START_VALUE;
		break;
		case CW:
			m_motor_ctrl.iq_start_value = -IQ_START_VALUE;
		break;
	}
	
}

