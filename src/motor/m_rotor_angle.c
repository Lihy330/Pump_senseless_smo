#include <stdio.h>
#include "m_rotor_angle.h"
#include "m_parameter.h"
#include "../drv/drv_hall.h"
#include "../drv/drv_hall_cb.h"
#include "m_ctrl.h"
#include "../math/math_utils.h"
#include "m_foc.h"
#include "m_observer.h"


// 一定注意，这个电角度与霍尔值的对应关系我们是需要测的，测量方法还没有学
static const uint16_t ROTOR_CCW_ANGLE[7] = {0, ANGLE_330, ANGLE_210, ANGLE_270, ANGLE_90, ANGLE_30, ANGLE_150};
static const uint16_t ROTOR_CW_ANGLE[7] = {0, ANGLE_30, ANGLE_270, ANGLE_330, ANGLE_150, ANGLE_90, ANGLE_210};
static const uint16_t ROTOR_INIT_ANGLE[7] = {0, ANGLE_0, ANGLE_240, ANGLE_300, ANGLE_120, ANGLE_60, ANGLE_180};

static union_u32 rotor_angle;
static union_u32 rotor_angle_inc;

m_obs_angle_unit_t m_obs_angle_unit;

typedef struct
{
	bsp_io_level_t u_value;
	bsp_io_level_t v_value;
	bsp_io_level_t w_value;
	uint8_t hall_value;
	uint8_t hall_last_value;
	uint32_t angle_60_time;
	uint32_t angle_temp_time;
	uint32_t angle_temp_time_last;
	uint32_t angle_60_time_filter1;
	uint32_t angle_60_time_filter2;
	uint8_t start_cnt;		// 电机运行起始阶段统计霍尔信号更新次数
	uint16_t update_cnt;	// 霍尔未更新的次数（时间），超过所设定的阈值抛异常
	bool start_sign;
	bool init_angle_sign;	// 是否是初始位置角
	bool update_sign;
}m_hall_unit_t;

m_hall_unit_t m_hall_unit;

void m_hall_get_value()
{
	uint8_t u_val, v_val, w_val;
	// 读取霍尔编码器
	R_IOPORT_PinRead(g_ioport.p_ctrl, HALL_U_IO_PORT, &m_hall_unit.u_value);
	R_IOPORT_PinRead(g_ioport.p_ctrl, HALL_V_IO_PORT, &m_hall_unit.v_value);
	R_IOPORT_PinRead(g_ioport.p_ctrl, HALL_W_IO_PORT, &m_hall_unit.w_value);

	u_val = (uint8_t)m_hall_unit.u_value;
	v_val = (uint8_t)m_hall_unit.v_value;
	w_val = (uint8_t)m_hall_unit.w_value;

	// 将三相霍尔传感器的值哈希映射
	m_hall_unit.hall_value = (uint8_t)((u_val << 2) | (v_val << 1) | w_val);
	// 判断当前霍尔值是否更新
	if ((m_hall_unit.hall_value > 0) && (m_hall_unit.hall_value < 7))
	{
		if (m_hall_unit.hall_last_value != m_hall_unit.hall_value)
		{
			m_hall_unit.hall_last_value = m_hall_unit.hall_value;
			m_hall_unit.update_sign = true;
		}
	}
	else
	{
//		printf("Hall value error!");
	}
}

void m_rotor_angle_init()
{
	m_hall_unit.u_value = 0;
	m_hall_unit.v_value = 0;
	m_hall_unit.w_value = 0;
	m_hall_unit.hall_value = 0;
	m_hall_unit.hall_last_value = 0;
	m_hall_unit.angle_60_time = 0;
	m_hall_unit.angle_temp_time = 0;
	m_hall_unit.angle_temp_time_last = 0;
	m_hall_unit.angle_60_time_filter1 = 0;
	m_hall_unit.angle_60_time_filter2 = 0;
	m_hall_unit.start_sign = true;
	m_hall_unit.update_sign = false;
	m_hall_unit.init_angle_sign = true;
	
	// 重置hall捕获值
//	drv_hall_capture.hall_reset_func();
	drv_hall_capture.hall_uvw_sign_clear_func();
	// 获取转子初始位置霍尔值
	m_hall_get_value();
	rotor_angle_inc.u32 = 0;
	// 获取转子初始位置角
	rotor_angle.u32 = ROTOR_INIT_ANGLE[m_hall_unit.hall_value];
}


uint16_t m_rotor_angle_calculate()
{
	uint32_t temp;
	m_hall_get_value();
	if (m_hall_unit.update_sign == true)
	{
		m_hall_unit.update_sign = false;
		m_hall_unit.update_cnt = 0;
		
		/************************************** 基于电机的旋转方向，确定当前转子位置角 *********************************************/
		switch(m_motor_ctrl.direction)
		{
			case CW:
				rotor_angle.u32 = ROTOR_CW_ANGLE[m_hall_unit.hall_value];
			break;
			case CCW:
				rotor_angle.u32 = ROTOR_CCW_ANGLE[m_hall_unit.hall_value];
			break;
		}
		
		/************************************** 根据霍尔信号跳变，计算60电角度执行时间 *********************************************/
		
		if (drv_hall_capture.u_sign || drv_hall_capture.v_sign || drv_hall_capture.w_sign)
		{
			// 霍尔u跳变
			if (drv_hall_capture.u_sign)
			{
				m_hall_unit.angle_temp_time = drv_hall_capture.u_capture_value;
			}
			// 霍尔v跳变
			else if (drv_hall_capture.v_sign)
			{
				m_hall_unit.angle_temp_time = drv_hall_capture.v_capture_value;
			}
			// 霍尔w跳变
			else if (drv_hall_capture.w_sign)
			{
				
				m_hall_unit.angle_temp_time = drv_hall_capture.w_capture_value;
			}
			// 清除霍尔标记
			drv_hall_capture.hall_uvw_sign_clear_func();
		}
		
		/*60°电角度时间计算*/
		if(m_hall_unit.angle_temp_time > m_hall_unit.angle_temp_time_last)
		{
			temp = m_hall_unit.angle_temp_time - m_hall_unit.angle_temp_time_last;
		}
		else
		{
			temp = 0xFFFFFFFF - m_hall_unit.angle_temp_time_last + m_hall_unit.angle_temp_time;
		}
		m_hall_unit.angle_temp_time_last = m_hall_unit.angle_temp_time;
		m_hall_unit.angle_temp_time = temp;
		
		if (m_hall_unit.angle_temp_time != 0)
		{
			m_hall_unit.angle_60_time_filter1 = LPF_CALC(m_hall_unit.angle_temp_time, \
													   m_hall_unit.angle_60_time_filter1);
			m_hall_unit.angle_60_time_filter2 = LPF_CALC(m_hall_unit.angle_60_time_filter1, \
													   m_hall_unit.angle_60_time_filter2);
		}
		
		// 稳定获取到10次霍尔信号更新之后，我们认为转速计算稳定
		if (m_motor_ctrl.speed_cal_stable_cnt ++  >= 12)
		{
			m_motor_ctrl.speed_cal_stable_cnt = 12;
			m_motor_ctrl.speed_cal_stable_flag = true;
			m_motor_ctrl.speed_update_flag = true;
		}
		// 获取到60°电角度的时间
		m_hall_unit.angle_60_time = m_hall_unit.angle_60_time_filter2;
		
		
		/************************************** 根据60°电角度时间计算δθ *********************************************/
		rotor_angle_inc.u32 = (uint32_t)((60.0f * 65536.0f * 50.0f) / (360.0f * (float)m_hall_unit.angle_60_time * 1.0f));
		// 记录一下当前转速
		m_motor_ctrl.speed = (uint16_t)(60000000 / (m_hall_unit.angle_60_time * 6 * POLAR));
	}
	else
	{
		if (m_hall_unit.update_cnt < HALL_VALUE_UPDATE_THRESHOLD_TIME)
		{
			m_hall_unit.update_cnt ++ ;
			// 根据电机旋转方向确定角度的更新
			switch(m_motor_ctrl.direction)
			{
				case CW:
					rotor_angle.u32 -= rotor_angle_inc.u32;
				break;
				case CCW:
					rotor_angle.u32 += rotor_angle_inc.u32;
				break;
			}
		}
		else
		{
			// 异常处理
			m_hall_unit.update_cnt = 0;
		}
//		printf("rotor_angle.u32: %d\r\n", rotor_angle.u32);
	}
	return (uint16_t)rotor_angle.words.low;
}


/*
	函数功能：初始化无感角度计算的相关参数
*/
void m_sensorless_angle_calculate_init()
{
	// delta_theta_e的低通滤波，一般将截止频率设置为300RPM
	/*
		RPM = 300 => RPS = 300 / 60 = 5;
		eRPS = fc = RPS * Polar = 10;
		所以 fc = 10;
		
		计算低通滤波的滤波系数 K = 2π * Tpwm * fc
		K = 2 * 3.1415926f * (1 / 20000) * 10 = 2 * 3.1415926f * 0.00005 * 10 = 0.0031415926
		K = 2 * 3.1415926f * (1 / 20000 * 20) * 10 = 2 * 3.1415926f * 0.00005 * 10 = 0.062831852 	?
		转换成Q15格式
		K = 0.0031415926 * 32768 = 103
		K = 0.062831852 * 32768 = 2060	?
	*/
	m_obs_angle_unit.q15_delta_theta_e_filter_k = DELTA_ANGLE_FILTER_K;
}


/*
	函数功能：计算反电动势与零度位置夹角theta_e，q15格式
	返回值：未补偿的theta_e，用于后续的角速度计算
	补偿的角度值存储在m_foc_unit结构体中
*/
void m_sensorless_theta_e_calculate()
{
	int16_t _q15_theta_e_uncompensated;
	
	_q15_theta_e_uncompensated = m_arctan_calculate(m_obs_unit.q15_e_alpha_n_estimation_final, m_obs_unit.q15_e_beta_n_estimation_final);
	
	// 基于当前的旋转方向进行相位补偿，我们在反电动势获取过程中进行了两次低通滤波（截止频率与反电动势频率相等），相位滞后了90°，在此处补偿
	switch(m_motor_ctrl.direction)
	{
		case CCW:
			/* 解释一下这里为什么q15与q16角度值能够直接相加减：
				反正切计算函数返回的虽然是q15格式的角度值，但是归一化的基准是π，所以打个比方，假设实际角度值为90°
					那么归一化之后是0.5，然后乘以32768，得到的是16384；
				而ANGLE_90归一化的基准是2π，因此，90°归一化之后为0.25，然后乘以65536得到的仍然是16384
				所以二者所代表的角度值是统一的，因此可以直接相加减
				而且，为什么不会溢出，假设计算出的未补偿的theta_e为180°，那么归一化之后的值为32768，此时加上ANGLE_90
					之后就会超出q15的最大值，但是溢出之后可以重新从q15的最小值-32767开始递增，因此溢出之后仍然是q15，仍然是-180°~180°
			*/
			m_obs_angle_unit.q15_theta_e_compensated = _q15_theta_e_uncompensated + ANGLE_90;
		break;
		case CW:
			m_obs_angle_unit.q15_theta_e_compensated = _q15_theta_e_uncompensated - ANGLE_90;
		break;
	}
	m_obs_angle_unit.q15_theta_e_uncompensated = _q15_theta_e_uncompensated;
}


/*
	函数功能：计算电机机械转速
	返回值：q15格式的机械转速
	角速度计算的采集周期是Tangle 一般设置为1ms
*/
void m_sensorless_rpm_calculate()
{
	int16_t erps_temp = 0;
	
	switch(m_motor_ctrl.direction)
	{
		case CCW:
			// 对每个pwm周期（50us）计算出的角度变化量求和
			// 这里如果上一次的角度值是170°，当前角度是-170°，那么理论上差值为20°，事实上也是这么执行的
			// -170°减去10°变为-180°，然后再减去160°，那么就从180°变成20°，刚好是差值，这个过程只要我们的差值依然保存在int16_t中，C语言就可以自动完成溢出后的处理
			m_obs_angle_unit.q15_delta_angle_Tangle_sum += m_obs_angle_unit.q15_theta_e_uncompensated - m_obs_angle_unit.q15_theta_e_uncompensated_last;
		break;
		case CW:
			m_obs_angle_unit.q15_delta_angle_Tangle_sum += m_obs_angle_unit.q15_theta_e_uncompensated_last - m_obs_angle_unit.q15_theta_e_uncompensated;
		break;
	}
	// 将当前的q15_theta_e_uncompensated_last用q15_theta_e_uncompensated更新
	m_obs_angle_unit.q15_theta_e_uncompensated_last = m_obs_angle_unit.q15_theta_e_uncompensated;
	
	m_obs_angle_unit.delta_angle_Tangle_cnt ++ ;
	
	// 如果当前累计累加了一个采集周期，那么就对当前的 q15_delta_angle_Tangle_sum 赋值给 q15_delta_angle_Tangle
	if (m_obs_angle_unit.delta_angle_Tangle_cnt >= DELTA_ANGLE_CNT)
	{	
		m_obs_angle_unit.q15_delta_angle_Tangle = m_obs_angle_unit.q15_delta_angle_Tangle_sum;
		
		m_obs_angle_unit.q15_delta_angle_Tangle_sum = 0;
		m_obs_angle_unit.delta_angle_Tangle_cnt = 0;
	}
	
	// 为了后续角度计算的平滑，需要对获取到的delta_angle进行低通滤波，设置截止频率为300RPM
	m_obs_angle_unit.q15_delta_angle_Tangle_filtered = m_obs_digital_LPF(m_obs_angle_unit.q15_delta_theta_e_filter_k, m_obs_angle_unit.q15_delta_angle_Tangle, m_obs_angle_unit.q15_delta_angle_Tangle_filtered);
	
	// 滤波完毕后计算转速
	// 当前角度值的含义：对于Q格式的数据来说，都是一个归一化的值，比如Q15的正值部分（0~32768）表示的就是（0~1）
		// 打个比方，当前的Q15数值是19660，对应的是0.6，就表示当前这个值是0.6倍的归一化基准值，也就是0.6个π
		// 		再通俗一点讲，就是0.6个半圈，然后引入采样周期这个概念，意义就是1个Tangle周期（假设是0.001s或者1ms）
		// 		转子旋转了0.6个半圈，代入0.001s，那么也就是说，转子在1s内旋转了0.6 / 0.001 = 600个半圈，也就是300圈
		// 		也就是说转速rps = (q15_delta_angle_Tangle / Tangle) / 32768 / 2，除以32768 / 2就相当于>>16
		// 		也就是说rps = (q15_delta_angle_Tangle / Tangle) >> 16
		// 我们就把Q格式的数据看做归一化的值就会很好理解了，只是不同的变量会有不同的归一化基准值
	
	
//	erps_temp = (int16_t)((int32_t)(m_obs_angle_unit.q15_delta_angle_Tangle_filtered * DELTA_ANGLE_TIME_INVERSE) >> 16);
//	// 对转速再次进行波峰波谷滤波
//	m_obs_angle_unit.erps_filter1 = LPF_CALC(erps_temp, m_obs_angle_unit.erps_filter1);
//	m_obs_angle_unit.erps_filter2 = LPF_CALC(m_obs_angle_unit.erps_filter1, m_obs_angle_unit.erps_filter2);
//	m_obs_angle_unit.erps = LPF_CALC(m_obs_angle_unit.erps_filter2, m_obs_angle_unit.erps);
//	
//	m_obs_angle_unit.erpm = m_obs_angle_unit.erps * 60;
//	m_obs_angle_unit.rps = m_obs_angle_unit.erps / POLAR;
//	m_obs_angle_unit.rpm = m_obs_angle_unit.rps * 60;
	
	// 这个地方一定要乘以60000，不能乘以1000，然后最后再乘以60，因为只乘以1000，得到的数值可能会小于65536，右移之后会变成0
	m_obs_angle_unit.erpm = \
				(int32_t)(m_obs_angle_unit.q15_delta_angle_Tangle_filtered * 1000 * 60) >> 16;
	
	if(m_obs_angle_unit.erpm < 0)
	{
		m_obs_angle_unit.erpm = 0;
	}
	m_obs_angle_unit.erpm_filter1 = LPF_CALC((uint16_t)m_obs_angle_unit.erpm, m_obs_angle_unit.erpm_filter1);
	m_obs_angle_unit.erpm_filter2 = LPF_CALC(m_obs_angle_unit.erpm_filter1, m_obs_angle_unit.erpm_filter2);	
	m_obs_angle_unit.rpm = m_obs_angle_unit.erpm_filter2 / POLAR;
}


/*
	函数功能：一阶低通滤波器自适应滤波系数计算

	K = 2π * Tpwm * fc
	fc = (δθ/360) / δT
	K = π * Tpwm * ((δθ/180) / δT) = ((π * Tpwm) / δT) * (δθ/180)
	K = ((π * Tpwm) / δT) * q15_delta_angle_Tangle_filtered
	把((π * Tpwm) / δT)看做一个系数Q16_LPF_SUB_K，变换为q16，乘完之后变成q31，再右移16位就得到了q15的自适应滤波系数
	从上面推导的公式看到(δθ/180)就是我们经过arctan反正切计算得到的q15角度值的变化量，已经在计算角速度的函数中获取到了

	300RPM对应的δθ计算 — MIN_FC_DELTA_THETA
	
*/
void m_sensorless_LPF_K_calculate()
{
	// 滤波之后的角度增量δθ
	int16_t val = m_obs_angle_unit.q15_delta_angle_Tangle_filtered;
	
	// 判断当前的角度增量是否低于阈值，这个阈值是最低转速300RPM所对应的δθ，低于δθ我们仍然采用300RPM转速下的滤波系数
	/*
		RPM = 300 => RPS = 300 / 60 = 5;
		eRPS = fc = RPS * Polar = 10;
		所以 fc = 10;
		
		fc = (δθ/360) / δT，其中fc=10
		fc * δT = (δθ/ 180) / 2
		fc * δT * 2 = (δθ/ 180) = 10 * 0.001 * 2 = 0.02
		(δθ/ 180) = 0.02
		转换到q15就是0.02 * 32768 = 655
		
		需要注意的是，我们为什么要凑出(δθ/ 180) ，因为我们通过反正切计算出的角度，归一化的基准就是180°（π）
		还是那句话，q格式数据就是一个归一化的数字，0.02表示什么呢，表示的就是0.02个半圈的角度值
	*/
	if (val < MIN_FC_DELTA_THETA)
	{
		val = MIN_FC_DELTA_THETA;
	}
	int32_t k_val = (int32_t)Q16_LPF_SUB_K * val;
	m_obs_unit.q15_klpf = (int16_t)(k_val >> 16);
}

/*
	函数功能：执行无感的反电动势与零度位置角计算的代码
	返回值：q15_theta_e_compensated 补偿之后的theta_e
*/
int16_t m_sensorless_theta_e_execute()
{
	// 通过反正切计算theta_e，并补偿相位
	m_sensorless_theta_e_calculate();
	// 基于未补偿的theta_e计算转速，并且对delta_theta_e进行滤波
	m_sensorless_rpm_calculate();
	// 基于滤波后的delta_theta_e计算一阶低通滤波器的自适应滤波系数
	m_sensorless_LPF_K_calculate();
	return m_obs_angle_unit.q15_theta_e_compensated;
}


