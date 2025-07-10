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
	
	// ADC采集的数值范围是 0~4096，因此需要将其左移4位变成Q16格式的数据
	q16_adc_val = (uint16_t)(adc_unit.speed_voltage.average_value << 4);
	
	// 将ADC值转换到转速范围内（0 - 2300 rad/min）
	q16_spd_set_val = (q16_adc_val * MAX_SPEED_VALUE) >> 16;
	
	// 对速度目标值进行限幅
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
	// d轴PI控制器
	m_id_pid_unit.PID_target_value = 0;
	m_id_pid_unit.PID_actual_value = m_current_cal_unit.q15_id;					// d轴电流实时检测值
//	m_current_cal_unit.q15_Ud = chenggong_series_pid_algorithm(&m_id_pid_unit);
	m_current_cal_unit.q15_Ud = series_pid_func(&m_id_pid_unit);
	
	#if 0		// 单独调试电流环代码
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
	
	// q轴PI控制器
	m_iq_pid_unit.PID_actual_value = m_current_cal_unit.q15_iq;
//	m_current_cal_unit.q15_Uq = chenggong_series_pid_algorithm(&m_iq_pid_unit);
	m_current_cal_unit.q15_Uq = series_pid_func(&m_iq_pid_unit);
}

void speed_pid_execute()
{
	// 转速稳定获取到(已经由开环强拖切换到闭环滑膜，并且θe与θr之间角度差值已经校准到90°)
	if (m_motor_ctrl.speed_cal_stable_flag == true)
	{
		// 斜坡加速，设置速度环的目标值
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
		
		// 对输出进行限幅
		switch(m_motor_ctrl.direction)
		{
			// 速度环输出的一定是正值，我们根据旋转的方向，添加正负
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


// 开环加速曲线设置
// αn = αn-1 + δα
// wn = wn-1 + αn
// θn = θn-1 + wn
// 开环强拖转子位置角更新
void m_drag_rotor_angle_update()
{
	if (m_tick_unit.drag_tick_cnt != 0)
	{
		return ;
	}
	m_tick_unit.drag_tick_cnt = DRAG_TICK_CNT;
	// 更新角加速度值
	m_foc_unit.m_drag_rotor_angle.q16_drag_acc_value = m_foc_unit.m_drag_rotor_angle.q16_drag_acc_value + DRAG_ACC_STEP;
	
	// 更新角速度值
	m_foc_unit.m_drag_rotor_angle.q16_drag_w_value = m_foc_unit.m_drag_rotor_angle.q16_drag_w_value_last + m_foc_unit.m_drag_rotor_angle.q16_drag_acc_value;
	if (m_foc_unit.m_drag_rotor_angle.q16_drag_w_value > Q16_DRAG_W_MAX_VALUE)
	{
		m_foc_unit.m_drag_rotor_angle.q16_drag_w_value = Q16_DRAG_W_MAX_VALUE;
	}
	m_foc_unit.m_drag_rotor_angle.q16_drag_w_value_last = m_foc_unit.m_drag_rotor_angle.q16_drag_w_value;
	
	// 更新转子位置角
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

// 开环强拖SVPWM生成
void m_drag_torque_set()
{
	// 基于电流闭环输出Us
	calculate_Us_thetaC_func();
	// 更新Us与0°位置的夹角
	switch (m_motor_ctrl.direction)
	{
		case CCW:
			m_foc_unit.Us_angle = m_foc_unit.rotor_angle + ANGLE_90 + m_foc_unit.theta_c;
		break;
		case CW:
			m_foc_unit.Us_angle = m_foc_unit.rotor_angle - ANGLE_90 - m_foc_unit.theta_c;
		break;
	}
	// 生成SVPWM
	m_svpwm_generate(m_foc_unit.Us, m_foc_unit.Us_angle);
}

// 开环强拖切换到闭环的条件判断
// 通过判断滑膜观测器的输出θe与开环强拖所设定的转子位置角θr之间的差值是否处于(60°, 120°)之间，进行开环到闭环的切换
int m_drag_to_close_judgement()
{
	uint16_t q16_theta_e_compensated_temp = 0;
	// 先将归一化基准值为180°的m_obs_angle_unit.q15_theta_e_compensated转换到q16格式，原来是-180~180，对小于0的值，现在加上一个65536变成0~360
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
	
	/*间隔5ms进行Iq电流校正*/
	if(m_tick_unit.iq_calib_tick_cnt == 0)
	{
		m_tick_unit.iq_calib_tick_cnt = IQ_CALIB_TICK_CNT;//强拖切换闭环tick时间电流校正周期
		
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
	
	// 闭环切换的前提是转速已经达到500RPM
	if (m_foc_unit.m_drag_rotor_angle.q16_drag_w_value >= Q16_DRAG_W_MAX_VALUE)
	{
			// 连续3次角度差值在60~120之间，即可切换闭环
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
	校准θe与θr之间的角度差值，使其慢慢变为90°
*/
int calib_theta_e_r_sub_angle()
{
	uint16_t q16_theta_e_compensated_temp = 0;
	// 先将归一化基准值为180°的m_obs_angle_unit.q15_theta_e_compensated转换到q16格式，原来是-180~180，对小于0的值，现在加上一个65536变成0~360
	if (m_obs_angle_unit.q15_theta_e_compensated < 0)
	{
		q16_theta_e_compensated_temp = m_obs_angle_unit.q15_theta_e_compensated + 65536;
	}
	else
	{
		q16_theta_e_compensated_temp = m_obs_angle_unit.q15_theta_e_compensated;
	}
	
	// 校准sub_angle值
	if (m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle < ANGLE_90)
	{
		m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle ++ ;
	}
	else if (m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle > ANGLE_90)
	{
		m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle -- ;
	}
	
	// 计算转子位置角
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
		case MOTOR_START:		// 切换到电机启动状态，执行自举电容充电
		{	
			// 设置充电时间
			m_tick_unit.boost_charging_tick_cnt = BOOST_CHARGING_TICK_CNT;
			motor_boost_charging();
		}
		break;
		case MOTOR_BOOST_CHARGING:
		{
			// 充电完毕后进行转子预定位
			if (!m_tick_unit.boost_charging_tick_cnt)
			{
				m_motor_ctrl.m_motor_execute_state_machine = MOTOR_ROTOR_PREPOS_START1;
				// 第一次预定位的时间
				m_tick_unit.rotor_prepos1_tick_cnt = ROTOR_PREPOS1_TICK_CNT;
			}
		}
		break;
		case MOTOR_ROTOR_PREPOS_START1:		// 第一次转子预定位
		{
			if (!m_tick_unit.rotor_prepos1_tick_cnt)
			{
				m_motor_ctrl.m_motor_execute_state_machine = MOTOR_ROTOR_PREPOS_END1;
			}
			m_foc_unit.Us = ROTOR_PREPOS_US1;
			// 先将转子强制定位到90°的位置
			m_foc_unit.Us_angle = ANGLE_90;
			// 生成SVPWM
			m_svpwm_generate(m_foc_unit.Us, m_foc_unit.Us_angle);
		}
		break;
		case MOTOR_ROTOR_PREPOS_END1:
		{
			m_motor_ctrl.m_motor_execute_state_machine = MOTOR_ROTOR_PREPOS_START2;
			// 第一次预定位的时间
			m_tick_unit.rotor_prepos2_tick_cnt = ROTOR_PREPOS2_TICK_CNT;
		}
		break;
		case MOTOR_ROTOR_PREPOS_START2:		// 第二次转子预定位
		{
			if (!m_tick_unit.rotor_prepos2_tick_cnt)
			{
				m_motor_ctrl.m_motor_execute_state_machine = MOTOR_ROTOR_PREPOS_END2;
			}
			m_foc_unit.Us = ROTOR_PREPOS_US2;
			// 先将转子强制定位到0°的位置
			m_foc_unit.Us_angle = ANGLE_0;
			// 生成SVPWM
			m_svpwm_generate(m_foc_unit.Us, m_foc_unit.Us_angle);
		}
		break;
		case MOTOR_ROTOR_PREPOS_END2:
		{	
			// 电机参数初始化
			motor_init();
			// 转子初始位置角获取
			m_rotor_angle_init();
			// 速度环pid参数初始化
			spd_pid_para_init();
			// 电流环pid参数初始化
			id_iq_pid_para_init();
			// 切换状态机为电机开环强拖状态
			m_motor_ctrl.m_motor_execute_state_machine = MOTOR_DRAG;
			
			// 初始化电机开环强拖的启动电流
			m_iq_pid_unit.PID_target_value = m_motor_ctrl.iq_start_value;
			// 初始化开环强拖的参数
			m_foc_unit.m_drag_rotor_angle.q16_drag_acc_value = 0;
			m_foc_unit.m_drag_rotor_angle.q16_drag_acc_value_last = 0;
			m_foc_unit.m_drag_rotor_angle.q16_drag_w_value = 0;
			m_foc_unit.m_drag_rotor_angle.q16_drag_w_value_last = 0;
		}
		break;
		case MOTOR_DRAG:		// 电机强拖状态
		{
			/*
				电机强拖状态时，我们通过设置递增的角加速度，不断开环强拖电机使其旋转到反电动势特征明显的状态
			*/
			// 更新转子位置角(设置开环加速曲线)
			m_drag_rotor_angle_update();
			// 设置电机开环强拖的扭矩
			m_drag_torque_set();
			// 判断闭环切换条件
			if (m_drag_to_close_judgement())
			{
				m_motor_ctrl.m_motor_execute_state_machine = MOTOR_EXECUTE;
				// 给速度环一个初始的转速设定值
				m_foc_unit.q16_spd_set_value = MIN_SPEED_VALUE;
			}
		}
		break;
		case MOTOR_EXECUTE:
		{	
			#if 0	// 单独调试电流环
				m_iq_pid_unit.PID_target_value = iq_target;
			#endif
			// 基于电流环的输出，得到超前角以及Us矢量的模长
			chenggong_us_theta_c_calculate();
			// 速度环执行
			if (!m_tick_unit.spd_loop_tick_cnt)
			{
				m_tick_unit.spd_loop_tick_cnt = SPEED_LOOP_TICK_CNT;
				speed_pid_execute();
			}
			// 根据当前的旋转方向计算Us_angle
			switch(m_motor_ctrl.direction)
			{
				case CCW:
					m_foc_unit.Us_angle = m_foc_unit.rotor_angle + ANGLE_90 + m_foc_unit.theta_c;
				break;
				case CW:
					m_foc_unit.Us_angle = m_foc_unit.rotor_angle - ANGLE_90 - m_foc_unit.theta_c;
				break;
			}
			// 生成SVPWM
			m_svpwm_generate(m_foc_unit.Us, m_foc_unit.Us_angle);
			
			/* 
				校正θe与开环强拖所设定的转子位置角θr之间的差值m_foc_unit.m_drag_rotor_angle.q16_theta_e_r_sub_angle
				因为我们闭环切换时是根据这个差值是否连续3次处于60~120区间内，所以θe并不是严格超前θr 90°，因此需要慢慢将这个差值调整到90°
				同时计算当前的转子位置角，供下个周期使用，并且校正完毕之后再开启速度环即可
			*/
			if (calib_theta_e_r_sub_angle())
			{
				// 校正完毕之后开启速度环
				m_motor_ctrl.speed_cal_stable_flag = true;
			}
		}
		break;
		case MOTOR_TEST:
			m_foc_unit.Us = 16384;
			m_foc_unit.Us_angle += 50;
			// 生成SVPWM
			m_svpwm_generate(m_foc_unit.Us, m_foc_unit.Us_angle);
	}
}

