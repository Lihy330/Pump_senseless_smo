#include <stdio.h>
#include "m_foc.h"
#include "../drv/drv_adc_cb.h"
#include "m_parameter.h"
#include "m_tick.h"
#include "m_ctrl.h"
#include "m_rotor_angle.h"
#include "m_svpwm.h"
#include "m_pid.h"
#include "m_current_cal.h"

#define SLOPE_H_VALUE			100
#define SLOPE_L_VALUE			10

m_foc_unit_t m_foc_unit;

int16_t iq_target = 512;

void m_calculate_Us()
{
	uint16_t q16_adc_val = 0;
	uint16_t q16_spd_set_val = 0;
	
	// ADC�ɼ�����ֵ��Χ�� 0~4096�������Ҫ��������4λ���Q16��ʽ������
	q16_adc_val = (uint16_t)(adc_unit.speed_voltage.average_value << 4);
	
	// ��ADCֵת����ת�ٷ�Χ�ڣ�0 - 2300 rad/min��
	q16_spd_set_val = (q16_adc_val * MAX_SPEED_VALUE) >> 16;
	
	// ���ٶ�Ŀ��ֵ�����޷�
	if (q16_spd_set_val > MAX_SPEED_VALUE)
	{
		q16_spd_set_val = MAX_SPEED_VALUE;
	}
	else if (q16_spd_set_val < MIN_SPEED_VALUE)
	{
		q16_spd_set_val = 0;
	}

	m_foc_unit.q16_spd_set_value = q16_spd_set_val;
}

void manul_set_target_speed_func(uint16_t manul_spd)
{
	if (manul_spd > MAX_SPEED_VALUE)
	{
		manul_spd = MAX_SPEED_VALUE;
	}
	else if (manul_spd < MIN_SPEED_VALUE)
	{
		manul_spd = 0;
	}
	m_foc_unit.q16_spd_set_value = manul_spd;
}


void current_pid_execute()
{
	// d��PI������
	m_id_pid_unit.PID_target_value = 0;
	m_id_pid_unit.PID_actual_value = m_current_cal_unit.q15_id;					// d�����ʵʱ���ֵ
//	m_current_cal_unit.q15_Ud = chenggong_series_pid_algorithm(&m_id_pid_unit);
	m_current_cal_unit.q15_Ud = series_pid_func(&m_id_pid_unit);
	
	#if 0		// �������Ե���������
		switch(m_motor_ctrl.direction)
		{
			case CCW:
				m_iq_pid_unit.PID_target_value = iq_target;
			break;
			case CW:
				m_iq_pid_unit.PID_target_value = -iq_target;
			break;
		}
		
	#endif
	
	// q��PI������
	m_iq_pid_unit.PID_actual_value = m_current_cal_unit.q15_iq;
//	m_current_cal_unit.q15_Uq = chenggong_series_pid_algorithm(&m_iq_pid_unit);
	m_current_cal_unit.q15_Uq = series_pid_func(&m_iq_pid_unit);
}

void speed_pid_execute()
{
	// ת���ȶ���ȡ��(�Ѿ��ɿ���ǿ���л����ջ���Ĥ�����Ҧ�e���r֮��ǶȲ�ֵ�Ѿ�У׼��90��)
	if (m_motor_ctrl.speed_cal_stable_flag == true)
	{
		// б�¼��٣������ٶȻ���Ŀ��ֵ
		if (m_foc_unit.q16_spd_set_value > m_foc_unit.q16_spd_target_value)
		{
			if (m_foc_unit.q16_spd_set_value - m_foc_unit.q16_spd_target_value > SLOPE_H_VALUE)
			{
				m_foc_unit.q16_spd_target_value += SLOPE_H_VALUE;
			}
			else if (m_foc_unit.q16_spd_set_value - m_foc_unit.q16_spd_target_value > SLOPE_L_VALUE)
			{
				m_foc_unit.q16_spd_target_value += SLOPE_L_VALUE;
			}
			else
			{
				m_foc_unit.q16_spd_target_value = m_foc_unit.q16_spd_set_value;
			}
		}
		else
		{
			if (m_foc_unit.q16_spd_target_value - m_foc_unit.q16_spd_set_value > SLOPE_H_VALUE)
			{
				m_foc_unit.q16_spd_target_value -= SLOPE_H_VALUE;
			}
			else if (m_foc_unit.q16_spd_target_value - m_foc_unit.q16_spd_set_value > SLOPE_L_VALUE)
			{
				m_foc_unit.q16_spd_target_value -= SLOPE_L_VALUE;
			}
			else
			{
				m_foc_unit.q16_spd_target_value = m_foc_unit.q16_spd_set_value;
			}
		}
	
		m_spd_pid_unit.PID_target_value = m_foc_unit.q16_spd_target_value;
		m_spd_pid_unit.PID_actual_value = m_obs_angle_unit.rpm;
		chenggong_parallel_position_pid_algorithm(&m_spd_pid_unit);
		
		// ����������޷�
		switch(m_motor_ctrl.direction)
		{
			// �ٶȻ������һ������ֵ�����Ǹ�����ת�ķ����������
			case CCW:
				if (m_spd_pid_unit.PID_output_value < IQ_MIN_VALUE)
				{
					m_iq_pid_unit.PID_target_value = IQ_MIN_VALUE;
				}
				else
				{
					m_iq_pid_unit.PID_target_value = m_spd_pid_unit.PID_output_value;
				}
			break;
			case CW:
				if (m_spd_pid_unit.PID_output_value < IQ_MIN_VALUE)
				{
					m_iq_pid_unit.PID_target_value = -IQ_MIN_VALUE;
				}
				else
				{
					m_iq_pid_unit.PID_target_value = -m_spd_pid_unit.PID_output_value;
				}
			break;
		}
	}
}


// ����������������
// ��n = ��n-1 + �Ħ�
// wn = wn-1 + ��n
// ��n = ��n-1 + wn
// ����ǿ��ת��λ�ýǸ���
void m_drag_rotor_angle_update()
{
	if (m_tick_unit.drag_tick_cnt != 0)
	{
		return ;
	}
	m_tick_unit.drag_tick_cnt = DRAG_TICK_CNT;
	// ���½Ǽ��ٶ�ֵ
	m_foc_unit.m_drag_rotor_angle.q16_drag_acc_value = m_foc_unit.m_drag_rotor_angle.q16_drag_acc_value + DRAG_ACC_STEP;
	
	// ���½��ٶ�ֵ
	m_foc_unit.m_drag_rotor_angle.q16_drag_w_value = m_foc_unit.m_drag_rotor_angle.q16_drag_w_value_last + m_foc_unit.m_drag_rotor_angle.q16_drag_acc_value;
	if (m_foc_unit.m_drag_rotor_angle.q16_drag_w_value > Q16_DRAG_W_MAX_VALUE)
	{
		m_foc_unit.m_drag_rotor_angle.q16_drag_w_value = Q16_DRAG_W_MAX_VALUE;
	}
	m_foc_unit.m_drag_rotor_angle.q16_drag_w_value_last = m_foc_unit.m_drag_rotor_angle.q16_drag_w_value;
	
	// ����ת��λ�ý�
	switch (m_motor_ctrl.direction)
	{
		case CCW:
		{
			m_foc_unit.rotor_angle += m_foc_unit.m_drag_rotor_angle.q16_drag_w_value;
		}
		break;
		case CW:
		{
			m_foc_unit.rotor_angle -= m_foc_unit.m_drag_rotor_angle.q16_drag_w_value;
		}
		break;
	}
}

// ����ǿ��SVPWM����
void m_drag_torque_set()
{
	// ���ڵ����ջ����Us
	calculate_Us_thetaC_func();
	// ����Us��0��λ�õļн�
	switch (m_motor_ctrl.direction)
	{
		case CCW:
			m_foc_unit.Us_angle = m_foc_unit.rotor_angle + ANGLE_90 + m_foc_unit.theta_c;
		break;
		case CW:
			m_foc_unit.Us_angle = m_foc_unit.rotor_angle - ANGLE_90 - m_foc_unit.theta_c;
		break;
	}
	// ����SVPWM
	m_svpwm_generate(m_foc_unit.Us, m_foc_unit.Us_angle);
}

// ����ǿ���л����ջ��������ж�
// ͨ���жϻ�Ĥ�۲����������e�뿪��ǿ�����趨��ת��λ�ýǦ�r֮��Ĳ�ֵ�Ƿ���(60��, 120��)֮�䣬���п������ջ����л�
int m_drag_to_close_judgement()
{
	uint16_t q16_theta_e_compensated_temp = 0;
	// �Ƚ���һ����׼ֵΪ180���m_obs_angle_unit.q15_theta_e_compensatedת����q16��ʽ��ԭ����-180~180����С��0��ֵ�����ڼ���һ��65536���0~360
	if (m_obs_angle_unit.q15_theta_e_compensated < 0)
	{
		q16_theta_e_compensated_temp = m_obs_angle_unit.q15_theta_e_compensated + 65536;
	}
	else
	{
		q16_theta_e_compensated_temp = m_obs_angle_unit.q15_theta_e_compensated;
	}
	switch(m_motor_ctrl.direction)
	{
		case CCW:
		{
			m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle = q16_theta_e_compensated_temp - m_foc_unit.rotor_angle;
		}
		break;
		case CW:
		{
			m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle = m_foc_unit.rotor_angle - q16_theta_e_compensated_temp;
		}
		break;
	}
	
	/*���5ms����Iq����У��*/
	if(m_tick_unit.iq_calib_tick_cnt == 0)
	{
		m_tick_unit.iq_calib_tick_cnt = IQ_CALIB_TICK_CNT;//ǿ���л��ջ�tickʱ�����У������
		
		switch(m_motor_ctrl.direction)
		{
			case CCW:
				(m_iq_pid_unit.PID_target_value > 0)?m_iq_pid_unit.PID_target_value-=1:m_iq_pid_unit.PID_target_value;
			break;
			case CW:
				(m_iq_pid_unit.PID_target_value < 0)?m_iq_pid_unit.PID_target_value+=1:m_iq_pid_unit.PID_target_value;
			break;
		}
	}
	
	// �ջ��л���ǰ����ת���Ѿ��ﵽ500RPM
	if (m_foc_unit.m_drag_rotor_angle.q16_drag_w_value >= Q16_DRAG_W_MAX_VALUE)
	{
			// ����3�νǶȲ�ֵ��60~120֮�䣬�����л��ջ�
		if ((m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle > ANGLE_60) && (m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle < ANGLE_120))
		{
			m_foc_unit.m_drag_rotor_angle.open_to_close_cnt ++ ;
			if (m_foc_unit.m_drag_rotor_angle.open_to_close_cnt >= OPEN_TO_CLOSE_CNT_THRESHOLD)
			{
				m_foc_unit.m_drag_rotor_angle.open_to_close_cnt = 0;
				return 1;
			}
		}
		else
		{
			m_foc_unit.m_drag_rotor_angle.open_to_close_cnt = 0;
		}
	}
	return 0;
}

/*
	У׼��e���r֮��ĽǶȲ�ֵ��ʹ��������Ϊ90��
*/
int calib_theta_e_r_sub_angle()
{
	uint16_t q16_theta_e_compensated_temp = 0;
	// �Ƚ���һ����׼ֵΪ180���m_obs_angle_unit.q15_theta_e_compensatedת����q16��ʽ��ԭ����-180~180����С��0��ֵ�����ڼ���һ��65536���0~360
	if (m_obs_angle_unit.q15_theta_e_compensated < 0)
	{
		q16_theta_e_compensated_temp = m_obs_angle_unit.q15_theta_e_compensated + 65536;
	}
	else
	{
		q16_theta_e_compensated_temp = m_obs_angle_unit.q15_theta_e_compensated;
	}
	
	// У׼sub_angleֵ
	if (m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle < ANGLE_90)
	{
		m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle ++ ;
	}
	else if (m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle > ANGLE_90)
	{
		m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle -- ;
	}
	
	// ����ת��λ�ý�
	switch(m_motor_ctrl.direction)
	{
		case CCW:
		{
			m_foc_unit.rotor_angle = q16_theta_e_compensated_temp - m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle;
		}
		break;
		case CW:
		{
			m_foc_unit.rotor_angle = q16_theta_e_compensated_temp + m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle;
		}
		break;
	}
	if (m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle == ANGLE_90)
	{
		
		return 1;
	}
	return 0;
}

void m_foc_algorithm_execute()
{
	switch(m_motor_ctrl.m_motor_execute_state_machine)
	{
		case MOTOR_STOP:
		{
			motor_stop();
		}
		break;
		case MOTOR_START:		// �л����������״̬��ִ���Ծٵ��ݳ��
		{	
			// ���ó��ʱ��
			m_tick_unit.boost_charging_tick_cnt = BOOST_CHARGING_TICK_CNT;
			motor_boost_charging();
		}
		break;
		case MOTOR_BOOST_CHARGING:
		{
			// �����Ϻ����ת��Ԥ��λ
			if (!m_tick_unit.boost_charging_tick_cnt)
			{
				m_motor_ctrl.m_motor_execute_state_machine = MOTOR_ROTOR_PREPOS_START1;
				// ��һ��Ԥ��λ��ʱ��
				m_tick_unit.rotor_prepos1_tick_cnt = ROTOR_PREPOS1_TICK_CNT;
			}
		}
		break;
		case MOTOR_ROTOR_PREPOS_START1:		// ��һ��ת��Ԥ��λ
		{
			if (!m_tick_unit.rotor_prepos1_tick_cnt)
			{
				m_motor_ctrl.m_motor_execute_state_machine = MOTOR_ROTOR_PREPOS_END1;
			}
			m_foc_unit.Us = ROTOR_PREPOS_US1;
			// �Ƚ�ת��ǿ�ƶ�λ��90���λ��
			m_foc_unit.Us_angle = ANGLE_90;
			// ����SVPWM
			m_svpwm_generate(m_foc_unit.Us, m_foc_unit.Us_angle);
		}
		break;
		case MOTOR_ROTOR_PREPOS_END1:
		{
			m_motor_ctrl.m_motor_execute_state_machine = MOTOR_ROTOR_PREPOS_START2;
			// ��һ��Ԥ��λ��ʱ��
			m_tick_unit.rotor_prepos2_tick_cnt = ROTOR_PREPOS2_TICK_CNT;
		}
		break;
		case MOTOR_ROTOR_PREPOS_START2:		// �ڶ���ת��Ԥ��λ
		{
			if (!m_tick_unit.rotor_prepos2_tick_cnt)
			{
				m_motor_ctrl.m_motor_execute_state_machine = MOTOR_ROTOR_PREPOS_END2;
			}
			m_foc_unit.Us = ROTOR_PREPOS_US2;
			// �Ƚ�ת��ǿ�ƶ�λ��0���λ��
			m_foc_unit.Us_angle = ANGLE_0;
			// ����SVPWM
			m_svpwm_generate(m_foc_unit.Us, m_foc_unit.Us_angle);
		}
		break;
		case MOTOR_ROTOR_PREPOS_END2:
		{	
			// ���������ʼ��
			motor_init();
			// ת�ӳ�ʼλ�ýǻ�ȡ
			m_rotor_angle_init();
			// �ٶȻ�pid������ʼ��
			spd_pid_para_init();
			// ������pid������ʼ��
			id_iq_pid_para_init();
			// �л�״̬��Ϊ�������ǿ��״̬
			m_motor_ctrl.m_motor_execute_state_machine = MOTOR_DRAG;
			
			// ��ʼ���������ǿ�ϵ���������
			m_iq_pid_unit.PID_target_value = m_motor_ctrl.iq_start_value;
			// ��ʼ������ǿ�ϵĲ���
			m_foc_unit.m_drag_rotor_angle.q16_drag_acc_value = 0;
			m_foc_unit.m_drag_rotor_angle.q16_drag_acc_value_last = 0;
			m_foc_unit.m_drag_rotor_angle.q16_drag_w_value = 0;
			m_foc_unit.m_drag_rotor_angle.q16_drag_w_value_last = 0;
		}
		break;
		case MOTOR_DRAG:		// ���ǿ��״̬
		{
			/*
				���ǿ��״̬ʱ������ͨ�����õ����ĽǼ��ٶȣ����Ͽ���ǿ�ϵ��ʹ����ת�����綯���������Ե�״̬
			*/
			// ����ת��λ�ý�(���ÿ�����������)
			m_drag_rotor_angle_update();
			// ���õ������ǿ�ϵ�Ť��
			m_drag_torque_set();
			// �жϱջ��л�����
			if (m_drag_to_close_judgement())
			{
				m_motor_ctrl.m_motor_execute_state_machine = MOTOR_EXECUTE;
				// ���ٶȻ�һ����ʼ��ת���趨ֵ
				m_foc_unit.q16_spd_set_value = MIN_SPEED_VALUE;
			}
		}
		break;
		case MOTOR_EXECUTE:
		{	
			#if 0	// �������Ե�����
				m_iq_pid_unit.PID_target_value = iq_target;
			#endif
			// ���ڵ�������������õ���ǰ���Լ�Usʸ����ģ��
			chenggong_us_theta_c_calculate();
			// �ٶȻ�ִ��
			if (!m_tick_unit.spd_loop_tick_cnt)
			{
				m_tick_unit.spd_loop_tick_cnt = SPEED_LOOP_TICK_CNT;
				speed_pid_execute();
			}
			// ���ݵ�ǰ����ת�������Us_angle
			switch(m_motor_ctrl.direction)
			{
				case CCW:
					m_foc_unit.Us_angle = m_foc_unit.rotor_angle + ANGLE_90 + m_foc_unit.theta_c;
				break;
				case CW:
					m_foc_unit.Us_angle = m_foc_unit.rotor_angle - ANGLE_90 - m_foc_unit.theta_c;
				break;
			}
			// ����SVPWM
			m_svpwm_generate(m_foc_unit.Us, m_foc_unit.Us_angle);
			
			/* 
				У����e�뿪��ǿ�����趨��ת��λ�ýǦ�r֮��Ĳ�ֵm_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle
				��Ϊ���Ǳջ��л�ʱ�Ǹ��������ֵ�Ƿ�����3�δ���60~120�����ڣ����Ԧ�e�������ϸ�ǰ��r 90�㣬�����Ҫ�����������ֵ������90��
				ͬʱ���㵱ǰ��ת��λ�ýǣ����¸�����ʹ�ã�����У�����֮���ٿ����ٶȻ�����
			*/
			if (calib_theta_e_r_sub_angle())
			{
				// У�����֮�����ٶȻ�
				m_motor_ctrl.speed_cal_stable_flag = true;
			}
		}
		break;
		case MOTOR_TEST:
			m_foc_unit.Us = 16384;
			m_foc_unit.Us_angle += 50;
			// ����SVPWM
			m_svpwm_generate(m_foc_unit.Us, m_foc_unit.Us_angle);
	}
}

