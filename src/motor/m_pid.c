#include <stdio.h>
#include "m_pid.h"
#include "m_current_cal.h"
#include "m_parameter.h"
#include "m_tick.h"
#include "m_ctrl.h"
#include "../math/math_trigonometric.h"
#include "../drv/drv_adc_cb.h"
#include "arm_math.h"
#include "m_foc.h"

m_pid_unit_t m_id_pid_unit;
m_pid_unit_t m_iq_pid_unit;
m_pid_unit_t m_spd_pid_unit;


void id_iq_pid_para_init()
{
	memset(&m_id_pid_unit, 0, sizeof(m_id_pid_unit));
	memset(&m_iq_pid_unit, 0, sizeof(m_iq_pid_unit));
	m_id_pid_unit.kp = 32767;
	m_id_pid_unit.ki = 6000;
	m_id_pid_unit.kd = 0;
	
	m_id_pid_unit.PID_output_limit_value = 32767;
	
	m_iq_pid_unit.kp = 32767;
	m_iq_pid_unit.ki = 6000;
	m_iq_pid_unit.kd = 0;
	
	m_iq_pid_unit.PID_output_limit_value = 32767;
}


void spd_pid_para_init()
{
	memset(&m_spd_pid_unit, 0, sizeof(m_spd_pid_unit));
	
	m_spd_pid_unit.kp = 16384;
	m_spd_pid_unit.ki = 100;
	m_spd_pid_unit.kd = 40;
	
	m_spd_pid_unit.PID_output_limit_value = 32767;
}


int16_t parallel_position_pid_func(m_pid_unit_t *pid)
{
	union_s32 temp_val;
	
	pid->error = pid->PID_target_value - pid->PID_actual_value; 	// Q15
	
	// Kp_value ����
	// Kp�ķ�Χ��0-1֮�䣬���ֵ��Χ��-32768-32768������Kp_value��ֵ��Ҫ��������޷�ֵ��Ҫת�ٲ�ﵽ30000rpm���ϣ������ǲ����ܵ�
	temp_val.s32 = (int32_t)(pid->error * pid->kp);	// Q15 * Q16 => Q31
	pid->kp_value = temp_val.words.high;	// ȡ����λQ15��ֵ
	
	// �Ի�������޷�ֵ���м��㣬�������ֵ�����ֵ��ȷ���ģ�����ͨ��ʹ�����ֵ���ֵ��ȥ�������ֵ�͵õ��˻�������޷�ֵ
	if (pid->kp_value > 0)
	{
		pid->ki_value_limit_value = pid->PID_output_limit_value - pid->kp_value;
	}
	else
	{
		pid->ki_value_limit_value = pid->PID_output_limit_value + pid->kp_value;
	}
	
	// Ki_value ����
	// Q16 * Q15 => Q31, Q31 >> 16 => Q15
	pid->ki_value += (int16_t)((int32_t)(pid->ki * pid->error) >> 16);
	
	// Kp_value > 0 һ����ζ�� Ki_value > m_spd_pid_unit.ki_value_limit_value�����������޷���û�����
	if (pid->ki_value > pid->ki_value_limit_value)
	{
		pid->ki_value = pid->ki_value_limit_value;
	}
	if (pid->ki_value < -pid->ki_value_limit_value)
	{
		pid->ki_value = -pid->ki_value_limit_value;
	}
	
	// Kd_value ����
	temp_val.s32 = (int32_t)((pid->error - pid->last_error) * pid->kd);	// Q15 * Q16 => Q31
	pid->kd_value = temp_val.words.high;
	pid->last_error = pid->error;
	
	// ���ֵ����
	pid->PID_output_value = pid->kp_value + pid->ki_value;
	
	// ���ֵ�޷�
	if (pid->PID_output_value > pid->PID_output_limit_value)
	{
		pid->PID_output_value = pid->PID_output_limit_value;
	}
	if (pid->PID_output_value < -pid->PID_output_limit_value)
	{
		pid->PID_output_value = -pid->PID_output_limit_value;
	}
	return pid->PID_output_value;
}


int16_t chenggong_parallel_position_pid_algorithm(m_pid_unit_t *pid) 
{
	union_s32 val;
	int16_t	  d_diff_val;
	
	/*���㵱ǰ���*/
	pid->error = pid->PID_target_value - pid->PID_actual_value;
	/*���������*/
	val.s32 = pid->kp * pid->error;  	//Q16*Q15=Q31
	pid->kp_value = val.words.high;		//�����������ֵ Q15
	
	/*������ֵ����޷�*/
	if(pid->kp_value >= 0)
	{
		/*
			PI�����=������+������+΢����
			PI�����ΪQ15��ʽ�ģ���ΧΪ-32768~+32767
			���۲��ַ�������������PI�������ĺ��ĵ��ڣ�������Ϊ������΢������ݾ�����ƶ������
			��ǰ�ȼ�����������ֵ����ô��-32768~+32767�������Χ�
			ȥ���������ʵʱ����ֵ����ôʣ�µ�ֵ���ǻ����������޷�ֵ
		*/
		val.words.high = pid->PID_output_limit_value - pid->kp_value;//32767-pid->q15_kp_value >=0
	}
	else
	{
		val.words.high = pid->PID_output_limit_value + pid->kp_value;//32767+pid->q15_kp_value >=0
	}
	
	val.words.low = 0;
	pid->s_i_limit.s32 = val.s32;			//�������޷�ֵ
	
	/*���������: Ki_sum_val += Ki*error    Ki_sum_val = Ki*(error1+error2+.....errorn)*/
	val.s32 = pid->ki * pid->error; //Q16*Q15=Q31
	pid->i_sum.s32 += val.s32;		  //Q31
	
	/*������������ֵ�޷�*/
	if(pid->i_sum.s32 > pid->s_i_limit.s32)
	{
		pid->i_sum.s32 = pid->s_i_limit.s32;
	}
	/*�����������Сֵ�޷�*/
	if(pid->i_sum.s32 < -pid->s_i_limit.s32)
	{
		pid->i_sum.s32 = -pid->s_i_limit.s32;
	}
	pid->ki_value = pid->i_sum.words.high; //�����������ֵ
	
	/*΢�������*/
	d_diff_val = pid->error - pid->last_error; //΢�����ֵ
	pid->last_error = pid->error;			   //�ϴ����ֵ����
	val.s32 = pid->kd * d_diff_val;			   //Q16*Q15=Q31
	pid->kd_value = val.words.high;			   //΢���������ֵ
	
	/*Kp Ki Kd�����*/
	val.s32 = (int32_t)pid->kp_value + (int32_t)pid->ki_value + (int32_t)pid->kd_value;
	
	/*������ֵ�޷�*/
	if(val.s32 > pid->PID_output_limit_value)
	{
		pid->PID_output_value = pid->PID_output_limit_value;
	}
	/*�����Сֵ�޷�*/
	if(val.s32 < -pid->PID_output_limit_value)
	{
		pid->PID_output_value = -pid->PID_output_limit_value;
	}
	/*���������Χ��*/
	else
	{
		pid->PID_output_value = pid->kp_value + pid->ki_value + pid->kd_value;
	}
	
	return pid->PID_output_value;
}




// ������PID
int16_t series_pid_func(m_pid_unit_t *pid)
{
	union_s32 temp_value;
	
	pid->error = pid->PID_target_value - pid->PID_actual_value;
	
	// Kp_value ����
	temp_value.s32 = (int32_t)(pid->kp * pid->error);
	pid->kp_value = temp_value.words.high;
	
	temp_value.s32 = 0;
	
	// Ki_value ����
	if (pid->kp_value > 0)
	{
		pid->ki_value_limit_value = pid->PID_output_limit_value - pid->kp_value;
	}
	else
	{
		pid->ki_value_limit_value = pid->PID_output_limit_value + pid->kp_value;
	}
	
	pid->i_sum.s32 += (pid->ki * pid->kp_value) >> 16;
	
	
	if (pid->i_sum.s32 > pid->ki_value_limit_value)
	{
		pid->i_sum.s32 = pid->ki_value_limit_value;
	}
	if (pid->i_sum.s32 < -pid->ki_value_limit_value)
	{
		pid->i_sum.s32 = -pid->ki_value_limit_value;
	}
	
	pid->ki_value = pid->i_sum.s32 ;
	
	// ���ֵ����
	pid->PID_output_value = pid->kp_value + pid->ki_value;
	
	// ���ֵ�޷�
	if (pid->PID_output_value > pid->PID_output_limit_value)
	{
		pid->PID_output_value = pid->PID_output_limit_value;
	}
	if (pid->PID_output_value < -pid->PID_output_limit_value)
	{
		pid->PID_output_value = -pid->PID_output_limit_value;
	}
	
	return pid->PID_output_value;
}

int16_t chenggong_series_pid_algorithm(m_pid_unit_t *pid) 
{	
	union_s32 val;
	
	/*���㵱ǰ���*/
	pid->error = pid->PID_target_value - pid->PID_actual_value;
	/*
	������PID��ʽ
	Kp_val = Kp*error
	Ki_val += Ki*Kp_val
	Out = Kp_val + Ki_val;
	*/
	/*���������*/
	val.s32 = pid->kp * pid->error;  	//Q16*Q15=Q31
	pid->kp_value = val.words.high;		//�����������ֵ
	
	/*������ֵ����޷�*/
	if(pid->kp_value >= 0)
	{
		/*
			PI�����=������+������+΢����
			PI�����ΪQ15��ʽ�ģ���ΧΪ-32768~+32767
			���۲��ַ�������������PI�������ĺ��ĵ��ڣ�������Ϊ������΢������ݾ�����ƶ������
			��ǰ�ȼ�����������ֵ����ô��-32768~+32767�������Χ�
			ȥ���������ʵʱ����ֵ����ôʣ�µ�ֵ���ǻ����������޷�ֵ
		*/
		val.words.high = pid->PID_output_limit_value - pid->kp_value;//32767-pid->q15_kp_value >=0
	}
	else
	{
		val.words.high = pid->PID_output_limit_value + pid->kp_value;//32767+pid->q15_kp_value >=0
	}
	
	val.words.low = 0;
	pid->s_i_limit.s32 = val.s32;  				//�������޷�ֵ
	
	/*���������*/
	val.s32 = pid->ki * pid->kp_value; 	//Q16*Q15=Q31
	pid->i_sum.s32 += val.s32;		  	   		//Q31
	
	/*������������ֵ�޷�*/
	if(pid->i_sum.s32 > pid->s_i_limit.s32)
	{
		pid->i_sum.s32 = pid->s_i_limit.s32;
	}
	/*�����������Сֵ�޷�*/
	if(pid->i_sum.s32 < -pid->s_i_limit.s32)
	{
		pid->i_sum.s32 = -pid->s_i_limit.s32;
	}
	pid->ki_value = pid->i_sum.words.high; //�����������ֵ
	
	/*Kp Ki�����*/
	val.s32 = (int32_t)pid->kp_value + (int32_t)pid->ki_value;
	/*������ֵ�޷�*/
	if(val.s32 > pid->PID_output_limit_value)
	{
		pid->PID_output_value = pid->PID_output_limit_value;
	}
	/*�����Сֵ�޷�*/
	if(val.s32 < -pid->PID_output_limit_value)
	{
		pid->PID_output_value = -pid->PID_output_limit_value;
	}
	/*���������Χ��*/
	else
	{
		pid->PID_output_value = pid->kp_value + pid->ki_value;
	}
	
	return pid->PID_output_value;
}

