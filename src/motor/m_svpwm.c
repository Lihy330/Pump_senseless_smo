#include <stdio.h>
#include "m_svpwm.h"
#include "../math/math_trigonometric.h"
#include "m_parameter.h"
#include "m_foc.h"

m_svpwm_unit_t m_svpwm_unit;

void m_get_sector(uint16_t theta)
{
	if ((theta >= ANGLE_0) && (theta < ANGLE_60)) m_svpwm_unit.sector = 1;
	else if ((theta >= ANGLE_60) && (theta < ANGLE_120)) m_svpwm_unit.sector = 2;
	else if ((theta >= ANGLE_120) && (theta < ANGLE_180)) m_svpwm_unit.sector = 3;
	else if ((theta >= ANGLE_180) && (theta < ANGLE_240)) m_svpwm_unit.sector = 4;
	else if ((theta >= ANGLE_240) && (theta < ANGLE_300)) m_svpwm_unit.sector = 5;
	else m_svpwm_unit.sector = 6;
}

// x = sina; y = (1/2)sina - (sqrt(3)/2)cosa; z = (1/2)sina + (sqrt(3)/2)cosa;
void m_xyz_calculate()
{
	union_s32 temp_value;
	
	m_svpwm_unit.generic_x = m_foc_unit.rotor_angle_sin_value_q15;
	// 为了规避掉浮点数的运算，所以将(sqrt(3)/2)转换成了Q16格式的数据，两者相乘得到Q31格式的数据
	// 我们再右移16位就能得到浮点数(sqrt(3)/2)直接与cosa相乘的结果，右移与直接取Q31数据类型的高16位等效
	temp_value.s32 = SQRT3DIV2 * m_foc_unit.rotor_angle_cos_value_q15;
	m_svpwm_unit.generic_y = (m_foc_unit.rotor_angle_sin_value_q15 >> 1) + temp_value.words.high;
	m_svpwm_unit.generic_z = (m_foc_unit.rotor_angle_sin_value_q15 >> 1) - temp_value.words.high;
}

void m_ta_tb_calculate(int16_t first_xyz, int16_t second_xyz, uint16_t Us)
{
	union_u32 q32_mt_value;
	union_s32 q31_mt_xyz_value;
	int16_t q15ta;
	int16_t q15tb;
	
	if (first_xyz < 0) first_xyz = 0;
	if (second_xyz < 0)	second_xyz = 0;
	
	q32_mt_value.u32 = Us * (uint16_t)PWM_PERIOD_VALUE;
	q31_mt_xyz_value.s32 = q32_mt_value.words.high * first_xyz;
	// 注意这个地方，我们是用Q16乘以了Q15得到了Q31，但是我们想要得到的是Q16，所以不能直接取q31_mt_xyz_value的高16位，因为高16位的首位是符号位，得到的依然是Q15
	// 因此，我们可以直接将q31_mt_xyz_value >> 15，得到Q16的ta和tb，或者取完q31_mt_xyz_value高16位之后再左移一位
	// 一定要明确，我们采用Q系列的数据格式只是为了规避掉浮点数的运算，所以一定要从计算的本质出发
	// q32_mt_value.words.high * first_xyz 这个计算式中first_xyz为Q15数据格式，也就是-1~1 => -32768~32767，这个计算式的实际含义是讲三角函数-1~1的数值
	// 乘到q32_mt_value.words.high这个数上，我们如果直接取高16位，相当于除以2^16（65536），得到的结果就与直接用-1~1的三角函数值与q32_mt_value.words.high相乘
	// 得到的结果不相同了，前后相差一倍，所以应当右移15位，也就是除以2^15（32768）
	q15ta = q31_mt_xyz_value.words.high;
	// 这里与上0x0000FFFF是为了防止之前是负数，首位是符号位，左移一位之后符号位位于了第16位，与上0将其抹掉，但其实q15ta一定是正数，因为first_xyz为正数
	m_svpwm_unit.q16ta = (uint16_t)((uint32_t)(q15ta << 1) & 0x0000FFFF);

	q31_mt_xyz_value.s32 = q32_mt_value.words.high * second_xyz;
	q15tb = q31_mt_xyz_value.words.high;
	// 这里与上0x0000FFFF是为了防止之前是负数，首位是符号位，左移一位之后符号位位于了第16位，与上0将其抹掉，但其实q15ta一定是正数，因为first_xyz为正数
	m_svpwm_unit.q16tb = (uint16_t)((uint32_t)(q15tb << 1) & 0x0000FFFF);
}

void tc_tb_taout_calculate()
{
	uint16_t q16ta_value = m_svpwm_unit.q16ta;
	uint16_t q16tb_value = m_svpwm_unit.q16tb;
	m_svpwm_unit.tcout = (((uint16_t)PWM_PERIOD_VALUE - q16ta_value - q16tb_value) >> 1) >> 1;
	m_svpwm_unit.tbout = (q16tb_value >> 1) + m_svpwm_unit.tcout;
	m_svpwm_unit.taout = (q16ta_value >> 1) + m_svpwm_unit.tbout;
}

void m_svpwm_duty_setting(uint16_t Us)
{
	// 根据不同的扇区确定占空比的设置
	switch(m_svpwm_unit.sector)
	{
		case 1:
			m_ta_tb_calculate(-m_svpwm_unit.generic_z, m_svpwm_unit.generic_x, Us);
			tc_tb_taout_calculate();
			m_svpwm_unit.u_duty_setting_value = m_svpwm_unit.taout;
			m_svpwm_unit.v_duty_setting_value = m_svpwm_unit.tbout;
			m_svpwm_unit.w_duty_setting_value = m_svpwm_unit.tcout;
		break;
		case 2:
			m_ta_tb_calculate(m_svpwm_unit.generic_z, m_svpwm_unit.generic_y, Us);
			tc_tb_taout_calculate();
			m_svpwm_unit.u_duty_setting_value = m_svpwm_unit.tbout;
			m_svpwm_unit.v_duty_setting_value = m_svpwm_unit.taout;
			m_svpwm_unit.w_duty_setting_value = m_svpwm_unit.tcout;
		break;
		case 3:
			m_ta_tb_calculate(m_svpwm_unit.generic_x, -m_svpwm_unit.generic_y, Us);
			tc_tb_taout_calculate();
			m_svpwm_unit.u_duty_setting_value = m_svpwm_unit.tcout;
			m_svpwm_unit.v_duty_setting_value = m_svpwm_unit.taout;
			m_svpwm_unit.w_duty_setting_value = m_svpwm_unit.tbout;
		break;
		case 4:
			m_ta_tb_calculate(-m_svpwm_unit.generic_x, m_svpwm_unit.generic_z, Us);
			tc_tb_taout_calculate();
			m_svpwm_unit.u_duty_setting_value = m_svpwm_unit.tcout;
			m_svpwm_unit.v_duty_setting_value = m_svpwm_unit.tbout;
			m_svpwm_unit.w_duty_setting_value = m_svpwm_unit.taout;
		break;
		case 5:
			m_ta_tb_calculate(-m_svpwm_unit.generic_y, -m_svpwm_unit.generic_z, Us);
			tc_tb_taout_calculate();
			m_svpwm_unit.u_duty_setting_value = m_svpwm_unit.tbout;
			m_svpwm_unit.v_duty_setting_value = m_svpwm_unit.tcout;
			m_svpwm_unit.w_duty_setting_value = m_svpwm_unit.taout;
		break;
		case 6:
			m_ta_tb_calculate(m_svpwm_unit.generic_y, -m_svpwm_unit.generic_x, Us);
			tc_tb_taout_calculate();
			m_svpwm_unit.u_duty_setting_value = m_svpwm_unit.taout;
			m_svpwm_unit.v_duty_setting_value = m_svpwm_unit.tcout;
			m_svpwm_unit.w_duty_setting_value = m_svpwm_unit.tbout;
		break;
	}
	// 对占空比进行限幅
	if (m_svpwm_unit.u_duty_setting_value < MIN_DUTY_VALUE) m_svpwm_unit.u_duty_setting_value = MIN_DUTY_VALUE;
	if (m_svpwm_unit.v_duty_setting_value < MIN_DUTY_VALUE) m_svpwm_unit.v_duty_setting_value = MIN_DUTY_VALUE;
	if (m_svpwm_unit.w_duty_setting_value < MIN_DUTY_VALUE) m_svpwm_unit.w_duty_setting_value = MIN_DUTY_VALUE;
	if (m_svpwm_unit.u_duty_setting_value > MAX_DUTY_VALUE) m_svpwm_unit.u_duty_setting_value = MAX_DUTY_VALUE;
	if (m_svpwm_unit.v_duty_setting_value > MAX_DUTY_VALUE) m_svpwm_unit.v_duty_setting_value = MAX_DUTY_VALUE;
	if (m_svpwm_unit.w_duty_setting_value > MAX_DUTY_VALUE) m_svpwm_unit.w_duty_setting_value = MAX_DUTY_VALUE;
	
	// 设置最终占空比生成SVPWM
	three_phase_duty_cycle_t three_phase_duty_cycle = {0};
	three_phase_duty_cycle.duty[0] = GPT_RELOAD_VALUE - m_svpwm_unit.u_duty_setting_value;
	three_phase_duty_cycle.duty[1] = GPT_RELOAD_VALUE - m_svpwm_unit.v_duty_setting_value;
	three_phase_duty_cycle.duty[2] = GPT_RELOAD_VALUE - m_svpwm_unit.w_duty_setting_value;

//	three_phase_duty_cycle.duty_buffer[0] = 0;
//	three_phase_duty_cycle.duty_buffer[1] = 0;
//	three_phase_duty_cycle.duty_buffer[2] = 0;

	R_GPT_THREE_PHASE_DutyCycleSet(g_three_phase0.p_ctrl, &three_phase_duty_cycle);
}


void m_u_alpha_beta_calculate(uint16_t Us)
{
	m_foc_unit.U_alpha_last = m_foc_unit.U_alpha;
	m_foc_unit.U_beta_last = m_foc_unit.U_beta;
	
	m_foc_unit.U_alpha = (int16_t)((int32_t)(Us * m_foc_unit.rotor_angle_cos_value_q15) >> 16);
	m_foc_unit.U_beta = (int16_t)((int32_t)(Us * m_foc_unit.rotor_angle_sin_value_q15) >> 16);
}

void m_svpwm_generate(uint16_t Us, uint16_t theta)
{
	union_s32 sin_cos_res;
	// 1. 计算Us与0轴夹角的正余弦值
	sin_cos_res = sin_cos_calculate(theta);
	
	m_foc_unit.rotor_angle_sin_value_q15 = sin_cos_res.words.high;
	m_foc_unit.rotor_angle_cos_value_q15 = sin_cos_res.words.low;
	
	// 2. 根据theta判断当前所在扇区
	m_get_sector(theta);
	// 3. 计算x y z的数值
	m_xyz_calculate();
	// 4. 计算tcout、tbout、taout的值，并填写GPT定时器的输出比较寄存器的数值，生成SVPWM方波
	m_svpwm_duty_setting(Us);
	// 5. 计算α与β轴的电压
	m_u_alpha_beta_calculate(Us);
}
