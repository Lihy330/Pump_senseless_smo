#include <stdio.h>
#include "m_ctrl.h"
#include "m_foc.h"
#include "m_parameter.h"

m_motor_ctrl_t m_motor_ctrl;


void motor_boost_charging()
{
	m_motor_ctrl.m_motor_execute_state_machine = MOTOR_BOOST_CHARGING;
	
	three_phase_duty_cycle_t three_phase_duty_cycle = {0};
	
	// �����Ϲܹرգ��¹ܿ������Ծٵ���Ԥ���
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
	
	// ��ȡ���еİ취��û�а취ʹ��GPT��ʱ��Disable����ΪADC�ɼ���ҪGPT��ʱ������������ֻ��ͨ�����Ϲ�ռ�ձ�����Ϊ0��ֹͣ���
	// ��ȫ�����һ�����ͣ�����ͽ���Դ�ϵ�����
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

// ��ʼ��������ص����в���
void motor_init()
{
	m_motor_ctrl.speed = 0;
	m_motor_ctrl.last_speed = 0;
	m_motor_ctrl.speed_cal_stable_flag = false;
	m_motor_ctrl.speed_update_flag = false;
	m_motor_ctrl.speed_cal_stable_cnt = 0;
	
	// ��ʼ��q����ʼ����
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

