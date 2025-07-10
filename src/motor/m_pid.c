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
	
	// Kp_value 计算
	// Kp的范围在0-1之间，输出值范围在-32768-32768，所以Kp_value的值想要超出输出限幅值需要转速差达到30000rpm以上，但这是不可能的
	temp_val.s32 = (int32_t)(pid->error * pid->kp);	// Q15 * Q16 => Q31
	pid->kp_value = temp_val.words.high;	// 取出高位Q15数值
	
	// 对积分项的限幅值进行计算，我们输出值的最大值是确定的，我们通过使用输出值最大值减去比例项的值就得到了积分项的限幅值
	if (pid->kp_value > 0)
	{
		pid->ki_value_limit_value = pid->PID_output_limit_value - pid->kp_value;
	}
	else
	{
		pid->ki_value_limit_value = pid->PID_output_limit_value + pid->kp_value;
	}
	
	// Ki_value 计算
	// Q16 * Q15 => Q31, Q31 >> 16 => Q15
	pid->ki_value += (int16_t)((int32_t)(pid->ki * pid->error) >> 16);
	
	// Kp_value > 0 一定意味着 Ki_value > m_spd_pid_unit.ki_value_limit_value，所以这样限幅是没问题的
	if (pid->ki_value > pid->ki_value_limit_value)
	{
		pid->ki_value = pid->ki_value_limit_value;
	}
	if (pid->ki_value < -pid->ki_value_limit_value)
	{
		pid->ki_value = -pid->ki_value_limit_value;
	}
	
	// Kd_value 计算
	temp_val.s32 = (int32_t)((pid->error - pid->last_error) * pid->kd);	// Q15 * Q16 => Q31
	pid->kd_value = temp_val.words.high;
	pid->last_error = pid->error;
	
	// 输出值计算
	pid->PID_output_value = pid->kp_value + pid->ki_value;
	
	// 输出值限幅
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
	
	/*计算当前误差*/
	pid->error = pid->PID_target_value - pid->PID_actual_value;
	/*比例项计算*/
	val.s32 = pid->kp * pid->error;  	//Q16*Q15=Q31
	pid->kp_value = val.words.high;		//比例项计算结果值 Q15
	
	/*积分项值最大限幅*/
	if(pid->kp_value >= 0)
	{
		/*
			PI的输出=比例项+积分项+微分项
			PI的输出为Q15格式的：范围为-32768~+32767
			理论部分分析：比例项是PI控制器的核心调节，积分项为辅助，微分项根据具体控制对象决定
			当前先计算出比例项的值，那么在-32768~+32767的输出范围里，
			去掉比例项的实时计算值，那么剩下的值就是积分项的最大限幅值
		*/
		val.words.high = pid->PID_output_limit_value - pid->kp_value;//32767-pid->q15_kp_value >=0
	}
	else
	{
		val.words.high = pid->PID_output_limit_value + pid->kp_value;//32767+pid->q15_kp_value >=0
	}
	
	val.words.low = 0;
	pid->s_i_limit.s32 = val.s32;			//积分项限幅值
	
	/*积分项计算: Ki_sum_val += Ki*error    Ki_sum_val = Ki*(error1+error2+.....errorn)*/
	val.s32 = pid->ki * pid->error; //Q16*Q15=Q31
	pid->i_sum.s32 += val.s32;		  //Q31
	
	/*积分项抗饱和最大值限幅*/
	if(pid->i_sum.s32 > pid->s_i_limit.s32)
	{
		pid->i_sum.s32 = pid->s_i_limit.s32;
	}
	/*积分项抗饱和最小值限幅*/
	if(pid->i_sum.s32 < -pid->s_i_limit.s32)
	{
		pid->i_sum.s32 = -pid->s_i_limit.s32;
	}
	pid->ki_value = pid->i_sum.words.high; //积分项计算结果值
	
	/*微分项计算*/
	d_diff_val = pid->error - pid->last_error; //微分项差值
	pid->last_error = pid->error;			   //上次误差值更新
	val.s32 = pid->kd * d_diff_val;			   //Q16*Q15=Q31
	pid->kd_value = val.words.high;			   //微分项计算结果值
	
	/*Kp Ki Kd输出和*/
	val.s32 = (int32_t)pid->kp_value + (int32_t)pid->ki_value + (int32_t)pid->kd_value;
	
	/*输出最大值限幅*/
	if(val.s32 > pid->PID_output_limit_value)
	{
		pid->PID_output_value = pid->PID_output_limit_value;
	}
	/*输出最小值限幅*/
	if(val.s32 < -pid->PID_output_limit_value)
	{
		pid->PID_output_value = -pid->PID_output_limit_value;
	}
	/*正常输出范围内*/
	else
	{
		pid->PID_output_value = pid->kp_value + pid->ki_value + pid->kd_value;
	}
	
	return pid->PID_output_value;
}




// 串联型PID
int16_t series_pid_func(m_pid_unit_t *pid)
{
	union_s32 temp_value;
	
	pid->error = pid->PID_target_value - pid->PID_actual_value;
	
	// Kp_value 计算
	temp_value.s32 = (int32_t)(pid->kp * pid->error);
	pid->kp_value = temp_value.words.high;
	
	temp_value.s32 = 0;
	
	// Ki_value 计算
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
	
	// 输出值计算
	pid->PID_output_value = pid->kp_value + pid->ki_value;
	
	// 输出值限幅
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
	
	/*计算当前误差*/
	pid->error = pid->PID_target_value - pid->PID_actual_value;
	/*
	串联型PID公式
	Kp_val = Kp*error
	Ki_val += Ki*Kp_val
	Out = Kp_val + Ki_val;
	*/
	/*比例项计算*/
	val.s32 = pid->kp * pid->error;  	//Q16*Q15=Q31
	pid->kp_value = val.words.high;		//比例项计算结果值
	
	/*积分项值最大限幅*/
	if(pid->kp_value >= 0)
	{
		/*
			PI的输出=比例项+积分项+微分项
			PI的输出为Q15格式的：范围为-32768~+32767
			理论部分分析：比例项是PI控制器的核心调节，积分项为辅助，微分项根据具体控制对象决定
			当前先计算出比例项的值，那么在-32768~+32767的输出范围里，
			去掉比例项的实时计算值，那么剩下的值就是积分项的最大限幅值
		*/
		val.words.high = pid->PID_output_limit_value - pid->kp_value;//32767-pid->q15_kp_value >=0
	}
	else
	{
		val.words.high = pid->PID_output_limit_value + pid->kp_value;//32767+pid->q15_kp_value >=0
	}
	
	val.words.low = 0;
	pid->s_i_limit.s32 = val.s32;  				//积分项限幅值
	
	/*积分项计算*/
	val.s32 = pid->ki * pid->kp_value; 	//Q16*Q15=Q31
	pid->i_sum.s32 += val.s32;		  	   		//Q31
	
	/*积分项抗饱和最大值限幅*/
	if(pid->i_sum.s32 > pid->s_i_limit.s32)
	{
		pid->i_sum.s32 = pid->s_i_limit.s32;
	}
	/*积分项抗饱和最小值限幅*/
	if(pid->i_sum.s32 < -pid->s_i_limit.s32)
	{
		pid->i_sum.s32 = -pid->s_i_limit.s32;
	}
	pid->ki_value = pid->i_sum.words.high; //积分项计算结果值
	
	/*Kp Ki输出和*/
	val.s32 = (int32_t)pid->kp_value + (int32_t)pid->ki_value;
	/*输出最大值限幅*/
	if(val.s32 > pid->PID_output_limit_value)
	{
		pid->PID_output_value = pid->PID_output_limit_value;
	}
	/*输出最小值限幅*/
	if(val.s32 < -pid->PID_output_limit_value)
	{
		pid->PID_output_value = -pid->PID_output_limit_value;
	}
	/*正常输出范围内*/
	else
	{
		pid->PID_output_value = pid->kp_value + pid->ki_value;
	}
	
	return pid->PID_output_value;
}

