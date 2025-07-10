#include <stdio.h>
#include "m_current_cal.h"
#include "m_parameter.h"
#include "m_tick.h"
#include "m_ctrl.h"
#include "../math/math_trigonometric.h"
#include "../drv/drv_adc_cb.h"
#include "arm_math.h"
#include "m_foc.h"
#include "m_pid.h"
#include "m_svpwm.h"


// 电流计算的总体思路
// ADC采集值范围 0-4095  （差分电压 * 10） + 1.65V + 静态误差
// 假设运放的静态误差的ADC值为100，那么我们在电机运转之前采集到的ADC值为运放的静态误差放大10倍之后的ADC值（100），加上1.65V的ADC值2048，也就是2148
// 电机运转之后，采集到（（差分电压 * 10） + 1.65V + 静态误差 * 10）的ADC值，假设是1148，我们减掉2148，得到-1000，那么这个时候得到的就是差分电压放大10倍之后的值
// 因此，我们减掉静态误差之后，数值范围变成了 -2048 -- 2048
// 如果我们减掉静态之后为0，那么说明当前的差分电压为0，如果减掉静态误差之后为-2048，说明当前差分电压为-165mV
// 因此，-2048对应差分电压为-1.65V，2048对应差分电压为1.65V
// 因此，我们将减掉静态误差放大10倍外加1.65V的ADC值之后，我们就得到了单纯的差分电压的ADC值，这个值的范围是-2048 -- 2048，相当于是Q12格式的数据（-4096 -- 4096）
// 将其左移3位变为Q15格式的数据（-32768 -- 32768）,但其实我们值的范围是 -16384 -- 16384，也就是-1.65V -- 1.65V
// 其实我们采集到的是电压值，但是可以代替电流值，因为二者只相差一个采样电阻阻值分之一，所以我们直接用采集到的电压值即可进行后续算法的实现


m_current_cal_unit_t m_current_cal_unit;


// 该函数功能是设置一个电机运转之前采集静态误差的时间，时间减到0之后，ADC中断处停止对静态误差的采集和计算，然后开始进入电机函数的执行
void current_static_error_cal()
{
	m_motor_ctrl.current_error_tick_flag = false;
	m_tick_unit.current_static_error_tick_cnt = CURRENT_STATIC_ERROR_TICK_CNT;
	
	while (1)
	{
		if (m_motor_ctrl.current_error_tick_flag == true)
		{
			break;
		}
	}
	
}

// 将采集到的三相实时电流值，减去静态误差得到真实的采样电阻两端的电流值（注意，实际上是采样电阻两端的电压值，与电流值就是一个采样电阻值分之一的倍数关系）
void get_tripple_phase_current()
{
	int16_t ia = 0;
	int16_t ib = 0;
	int16_t ic = 0;
	
	// 本来应该是用实时检测到的电流值减去静态误差值，但是我们正负号代表方向，所以直接用静态误差作为被减数，就不用乘以-1来调整电流值的方向了
	ia = (int16_t)((adc_unit.ia_static_error - adc_unit.u_current.instant_value) << 3);  // 差值是-2048 -- 2048 然后左移3位，变成Q15格式
	ib = (int16_t)((adc_unit.ib_static_error - adc_unit.v_current.instant_value) << 3);
	ic = (int16_t)((adc_unit.ic_static_error - adc_unit.w_current.instant_value) << 3);
	
	
		/*根据扇区编号：采样窗口小的电流根据KCL定律计算*/
	switch(m_svpwm_unit.sector)
	{
		case 1:
			ia = 0 - ib - ic;	//U相采样窗口最小,KCL定律计算采样窗口小电流
		break;
		case 2:
			ib = 0 - ia - ic;	//V相采样窗口最小,KCL定律计算采样窗口小电流
		break;
		case 3:
			ib = 0 - ia - ic;	//V相采样窗口最小,KCL定律计算采样窗口小电流
		break;
		case 4:
			ic = 0 - ia - ib;	//W相采样窗口最小,KCL定律计算采样窗口小电流
		break;
		case 5:
			ic = 0 - ia - ib;	//W相采样窗口最小,KCL定律计算采样窗口小电流
		break;
		case 6:
			ia = 0 - ib - ic;	//U相采样窗口最小,KCL定律计算采样窗口小电流
		break;
		default:
		break;
	}
	
	
	m_current_cal_unit.q15_ia = ia;
	m_current_cal_unit.q15_ib = ib;
	m_current_cal_unit.q15_ic = ic;
}

// Clark变换
void clark_transform()
{
	int16_t i_alpha = 0;
	union_s32 i_beta;
	union_s32 val_a;
	union_s32 val_b;

	i_alpha = m_current_cal_unit.q15_ia;	
	
	
	val_a.s32 = SQRT3DIV3 * m_current_cal_unit.q15_ia;
	val_b.s32 = SQRT3DIV3 * m_current_cal_unit.q15_ib;
	val_b.s32 = val_b.s32 * 2;
	i_beta.s32 = val_a.s32 + val_b.s32;
	// Q16 * Q15 = Q31
//	i_beta.s32 = (int32_t)(SQRT3DIV3 * (m_current_cal_unit.q15_ia + 2 * m_current_cal_unit.q15_ib));
	
	// 取Q31的高位
	m_current_cal_unit.q15_i_alpha = i_alpha;
	m_current_cal_unit.q15_i_beta = i_beta.words.high;
}

// Park变换
void park_transform(uint16_t theta)
{
	union_s32 sin_cos_res;
	int32_t id = 0;
	int32_t iq = 0;
	sin_cos_res = sin_cos_calculate(theta);
	// Q15 * Q15 = Q30
	id = ((m_current_cal_unit.q15_i_alpha * sin_cos_res.words.low) + (m_current_cal_unit.q15_i_beta * sin_cos_res.words.high));
	iq = ((m_current_cal_unit.q15_i_beta * sin_cos_res.words.low) - (m_current_cal_unit.q15_i_alpha * sin_cos_res.words.high));
	
	// Q30 -> Q15
	m_current_cal_unit.q15_id = (int16_t)(id >> 15);
	m_current_cal_unit.q15_iq = (int16_t)(iq >> 15);
}


// Us模长计算以及超前角计算
void calculate_Us_thetaC_func()
{
	uint16_t Us2;
	uint16_t Ud2;
	uint16_t Uq2_max;
	uint16_t Uq2;
	float Uq_max_f;
	float Us_f;
	float theta_c_rad_f;		// dsp计算出的弧度的超前角
	float theta_c_ang_f;
	int16_t Uq_max;
	
	/*******************************************************Us模长计算*******************************************************************/
	// 通过对速度环输出的Uq进行限幅，就可以实现对Us的限幅
	// 实际上Us应该是一个Q16格式的数据，但是我们要通过Us^2 - Ud^2得到Uq^2，而Uq是一个Q15的数据（它是有方向的，所以会有正有负），所以为了方便计算
	// 我们将Us_max这个限幅值设置为了Q15格式
	m_current_cal_unit.Us_max = Q15_US_MAX_VALUE;
	Us2 = (uint16_t)((uint32_t)(m_current_cal_unit.Us_max * m_current_cal_unit.Us_max) >> 14);	// Q15 * Q15 => Q30; Q30 >> 14 => Q16
	Ud2 = (uint16_t)((uint32_t)(m_current_cal_unit.q15_Ud * m_current_cal_unit.q15_Ud) >> 14);	// Q15 * Q15 => Q30; Q30 >> 14 => Q16
	
	// 计算Uq的最大限幅值的平方
	Uq2_max = Us2 - Ud2;
	
	// 先将Uq2_max转换成对应的浮点数，然后对Uq2_max进行开方运算
	arm_sqrt_f32((float)Uq2_max / 65536.0f, &Uq_max_f);
	Uq_max = (int16_t)((uint16_t)(Uq_max_f * 65536.0f) >> 1);
	
	// 对Uq进行限幅
	if (m_current_cal_unit.q15_Uq > Uq_max)
	{
		m_current_cal_unit.q15_Uq = Uq_max;
	}
	else if (m_current_cal_unit.q15_Uq < -Uq_max)
	{
		m_current_cal_unit.q15_Uq = -Uq_max;
	}
	
	// 计算Us
	Uq2 = (uint16_t)((uint32_t)(m_current_cal_unit.q15_Uq * m_current_cal_unit.q15_Uq) >> 14);	// Q15 * Q15 => Q30; Q30 >> 14 => Q16
	Us2 = Uq2 + Ud2;
	
	// 先将Us2转换成对应的浮点数，然后对Us2进行开方运算
	arm_sqrt_f32((float)(Us2 / 65536.0f), &Us_f);
	// 将浮点数转换成Q16格式的数据
	m_foc_unit.Us = (uint16_t)(Us_f * 65536.0f);
	
	/*******************************************************超前角计算*******************************************************************/
	// 判断Uq和Ud的值
	if (m_current_cal_unit.q15_Uq == 0)
	{
		m_foc_unit.theta_c = 0;
	}
	else if (m_current_cal_unit.q15_Ud == 0)
	{
		m_foc_unit.theta_c = 0;
	}
	else
	{
		
		switch(m_motor_ctrl.direction)
		{
			case CCW:
				// 逆时针时，Ud是负值，但是我们需要计算超前角的绝对值，所以需要将Ud乘以-1变换为正值
				arm_atan2_f32((float)((-1) * m_current_cal_unit.q15_Ud / 32768.0f), (float)(m_current_cal_unit.q15_Uq / 32768.0f), &theta_c_rad_f);
				// 将弧度制转为角度制
				theta_c_ang_f = theta_c_rad_f / PI * 180.0f;
				// 角度归一化
				theta_c_ang_f = theta_c_ang_f / 360.0f;
				m_foc_unit.theta_c = (uint16_t)(theta_c_ang_f * 65536.0f);
			break;
			case CW:
				arm_atan2_f32((float)(m_current_cal_unit.q15_Ud / 32768.0f), (float)(m_current_cal_unit.q15_Uq / 32768.0f), &theta_c_rad_f);
				// 将弧度制转为角度制
				theta_c_ang_f = theta_c_rad_f / PI * 180.0f;
				// 角度归一化
				theta_c_ang_f = theta_c_ang_f / 360.0f;
				m_foc_unit.theta_c = (uint16_t)(theta_c_ang_f * 65536.0f);
			break;
		}

	}
}


void chenggong_us_theta_c_calculate(void)
{
#if 0	
	uint16_t ud_2;
	uint16_t uq_2;
	uint16_t us_2;
	uint16_t us_max_2;
	float 	 us_f;
	/*Us模长最大限幅到90%*/
	m_foc_unit.q15_us_max = US_MAX_VALUE;
	
	/*
		Ud^2+Uq^2=Us^2
		Q15*Q15=Q30,Q30右移14位=Q16
	*/
	ud_2 = (uint16_t)((uint32_t)(m_foc_unit.coordinate.q15_ud * m_foc_unit.coordinate.q15_ud) >> 14);
	uq_2 = (uint16_t)((uint32_t)(m_foc_unit.coordinate.q15_uq * m_foc_unit.coordinate.q15_uq) >> 14);
	us_max_2 = (uint16_t)((uint32_t)(m_foc_unit.q15_us_max * m_foc_unit.q15_us_max) >> 14);
	
	/*如果Ud^2+Uq^2<=Us模长最大值的平方，正常计算*/
	if((ud_2 + uq_2) <= us_max_2)
	{
		us_2 = ud_2 + uq_2;
	}
	/*Us模长最大限幅*/
	else
	{
		us_2 = us_max_2;
	}
#else
	uint16_t ud_2;
	uint16_t uq_2;
	uint16_t uq_max_2;
	uint16_t us_2;
	uint16_t us_max_2;
	float	 uq_max_f;
	int16_t  uq_max;
	float 	 us_f;
	
	/*Us模长最大限幅到90%*/
	m_current_cal_unit.Us_max = Q15_US_MAX_VALUE;
	
	/*
		Ud^2+Uq^2=Us^2
		Q15*Q15=Q30,Q30右移14位=Q16
	*/
	ud_2 = (uint16_t)((uint32_t)(m_current_cal_unit.q15_Ud * m_current_cal_unit.q15_Ud) >> 14);
	us_max_2 = (uint16_t)((uint32_t)(m_current_cal_unit.Us_max * m_current_cal_unit.Us_max) >> 14);
	/*
		实际实现上，根据坐标投影的分析：认为Ud是在范围内，所以
		根据限制Uq的最大值，那么就根据uq_2 = us_max_2 - ud_2;这个公式计算出
		uq_max_2这个Uq最大值的平方
	*/
	uq_max_2 = us_max_2 - ud_2;
	
	/*
		(float)uq_2 / 65536.0f：将Q16格式的数据转换为对应的小数
		uq_max_f：开方运算后对应的Q16格式的小数
	*/
	arm_sqrt_f32((float)uq_max_2 / 65536.0f, &uq_max_f);
	/*
		uq_max_f * 65536.0f:Q16格式的Uq最大值
		((uint16_t)(uq_max_f * 65536.0f)) >> 1：右移1位转换为Q15格式的数据
		因为uq_max需要和Uq这个Q15格式的数据做比较
	*/
	uq_max = ((uint16_t)(uq_max_f * 65536.0f)) >> 1;
	
	/*Uq正的最大值限幅*/
	if(m_current_cal_unit.q15_Uq > uq_max)
	{
		m_current_cal_unit.q15_Uq = uq_max;
	}
	/*Uq负的最小值限幅*/
	if(m_current_cal_unit.q15_Uq < -uq_max)
	{
		m_current_cal_unit.q15_Uq = -uq_max;
	}
	
	/*
		Ud^2+Uq^2=Us^2
		Q15*Q15=Q30,Q30右移14位=Q16
	*/
	uq_2 = (uint16_t)((uint32_t)(m_current_cal_unit.q15_Uq * m_current_cal_unit.q15_Uq) >> 14);
	us_2 = ud_2 + uq_2;
#endif
	
	/*
		arm dsp库api接口进行开方计算
		将Q16格式的数据转换为小数形式的表达
		Us浮点类型的数据计算最大值为0.9
	*/
	arm_sqrt_f32((float)us_2 / 65536.0f, &us_f);
	/*计算Us模长：转换为Q16格式*/
	m_foc_unit.Us = (uint16_t)(us_f * 65536.0f);
	
	/*超前角计算：arctan(Ud/Uq)*/
	/*如果Uq=0，Ud/Uq这个运算规则不成立*/
	if(m_current_cal_unit.q15_Uq == 0)
	{
		m_foc_unit.theta_c = 0;//超前角直接设置为0°
	}
	/*如果Ud值等于0，说明Us完全落在q轴上，超前角为0°*/
	else if(m_current_cal_unit.q15_Ud == 0)
	{
		m_foc_unit.theta_c = 0;//超前角直接设置为0°
	}
	else
	{
		float x,y;
		y = (float)m_current_cal_unit.q15_Ud / 32768.0f;
		x = (float)m_current_cal_unit.q15_Uq / 32768.0f;
		float result_rad_f;
		float result_angle_f;
		
		#define PI_DIV_180			(180.0f / M_PI)
		#define RAD_TO_ANGLE(rad)	(float)(rad * PI_DIV_180)  //角度值 = 弧度值 × (180 / π)   
		
		
		switch(m_motor_ctrl.direction)
		{
			case CCW:
				/*
					Returned value is between -Pi and Pi
					逆时针旋转Uq为正，Ud为负
					因为将Ud和Uq都调整为了正值，计算结果在0~π之间(0°-180°)
					实际计算结果应该在0~π/2之间，也就是0°-90°之间
				*/						
				arm_atan2_f32(-y, x, &result_rad_f);
				/*弧度转角度*/
				result_angle_f = RAD_TO_ANGLE(result_rad_f);
				// 角度归一化
				result_angle_f = result_angle_f / 360.0f;
				m_foc_unit.theta_c = (uint16_t)(result_angle_f * 65536.0f); //将超前角转换为Q16格式
			break;
			case CW:
				/*
					Returned value is between -Pi and Pi
					顺时针旋转Uq为负，Ud为负
					因为将Ud和Uq都调整为了正值，计算结果在0~π之间(0°-180°)
					实际计算结果应该在0~π/2之间，也就是0°-90°之间
				*/						
				arm_atan2_f32(-y, -x, &result_rad_f);
				/*弧度转角度*/
				result_angle_f = RAD_TO_ANGLE(result_rad_f);
				// 角度归一化
				result_angle_f = result_angle_f / 360.0f;
				m_foc_unit.theta_c = (uint16_t)(result_angle_f * 65536.0f); //将超前角转换为Q16格式
			break;
		}
	}
}



void test_current_func()
{
	if (m_tick_unit.test_current_tick_cnt == 0)
	{
		printf("%d\t%d\t%d\r\n", m_current_cal_unit.q15_ia, m_current_cal_unit.q15_ib, m_current_cal_unit.q15_ic);
		printf("m_iq_pid_unit.PID_actual_value: %d\r\n", m_iq_pid_unit.PID_actual_value);
		printf("m_iq_pid_unit.PID_target_value: %d\r\n", m_iq_pid_unit.PID_target_value);
		printf("m_iq_pid_unit.PID_output_value: %d\r\n", m_iq_pid_unit.PID_output_value);
		printf("m_iq_pid_unit.error: %d\r\n", m_iq_pid_unit.error);
		printf("m_iq_pid_unit.kp_value: %d\r\n", m_iq_pid_unit.kp_value);
		printf("m_iq_pid_unit.ki_value: %d\r\n", m_iq_pid_unit.ki_value);
		printf("m_iq_pid_unit.kp: %d\r\n", m_iq_pid_unit.kp);
		printf("m_iq_pid_unit.ki: %d\r\n", m_iq_pid_unit.ki);
		m_tick_unit.test_current_tick_cnt = TEST_CURRENT_TICK_CNT;
	}
}


