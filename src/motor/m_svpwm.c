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
	// Ϊ�˹�ܵ������������㣬���Խ�(sqrt(3)/2)ת������Q16��ʽ�����ݣ�������˵õ�Q31��ʽ������
	// ����������16λ���ܵõ�������(sqrt(3)/2)ֱ����cosa��˵Ľ����������ֱ��ȡQ31�������͵ĸ�16λ��Ч
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
	// ע������ط�����������Q16������Q15�õ���Q31������������Ҫ�õ�����Q16�����Բ���ֱ��ȡq31_mt_xyz_value�ĸ�16λ����Ϊ��16λ����λ�Ƿ���λ���õ�����Ȼ��Q15
	// ��ˣ����ǿ���ֱ�ӽ�q31_mt_xyz_value >> 15���õ�Q16��ta��tb������ȡ��q31_mt_xyz_value��16λ֮��������һλ
	// һ��Ҫ��ȷ�����ǲ���Qϵ�е����ݸ�ʽֻ��Ϊ�˹�ܵ������������㣬����һ��Ҫ�Ӽ���ı��ʳ���
	// q32_mt_value.words.high * first_xyz �������ʽ��first_xyzΪQ15���ݸ�ʽ��Ҳ����-1~1 => -32768~32767���������ʽ��ʵ�ʺ����ǽ����Ǻ���-1~1����ֵ
	// �˵�q32_mt_value.words.high������ϣ��������ֱ��ȡ��16λ���൱�ڳ���2^16��65536�����õ��Ľ������ֱ����-1~1�����Ǻ���ֵ��q32_mt_value.words.high���
	// �õ��Ľ������ͬ�ˣ�ǰ�����һ��������Ӧ������15λ��Ҳ���ǳ���2^15��32768��
	q15ta = q31_mt_xyz_value.words.high;
	// ��������0x0000FFFF��Ϊ�˷�ֹ֮ǰ�Ǹ�������λ�Ƿ���λ������һλ֮�����λλ���˵�16λ������0����Ĩ��������ʵq15taһ������������Ϊfirst_xyzΪ����
	m_svpwm_unit.q16ta = (uint16_t)((uint32_t)(q15ta << 1) & 0x0000FFFF);

	q31_mt_xyz_value.s32 = q32_mt_value.words.high * second_xyz;
	q15tb = q31_mt_xyz_value.words.high;
	// ��������0x0000FFFF��Ϊ�˷�ֹ֮ǰ�Ǹ�������λ�Ƿ���λ������һλ֮�����λλ���˵�16λ������0����Ĩ��������ʵq15taһ������������Ϊfirst_xyzΪ����
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
	// ���ݲ�ͬ������ȷ��ռ�ձȵ�����
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
	// ��ռ�ձȽ����޷�
	if (m_svpwm_unit.u_duty_setting_value < MIN_DUTY_VALUE) m_svpwm_unit.u_duty_setting_value = MIN_DUTY_VALUE;
	if (m_svpwm_unit.v_duty_setting_value < MIN_DUTY_VALUE) m_svpwm_unit.v_duty_setting_value = MIN_DUTY_VALUE;
	if (m_svpwm_unit.w_duty_setting_value < MIN_DUTY_VALUE) m_svpwm_unit.w_duty_setting_value = MIN_DUTY_VALUE;
	if (m_svpwm_unit.u_duty_setting_value > MAX_DUTY_VALUE) m_svpwm_unit.u_duty_setting_value = MAX_DUTY_VALUE;
	if (m_svpwm_unit.v_duty_setting_value > MAX_DUTY_VALUE) m_svpwm_unit.v_duty_setting_value = MAX_DUTY_VALUE;
	if (m_svpwm_unit.w_duty_setting_value > MAX_DUTY_VALUE) m_svpwm_unit.w_duty_setting_value = MAX_DUTY_VALUE;
	
	// ��������ռ�ձ�����SVPWM
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
	// 1. ����Us��0��нǵ�������ֵ
	sin_cos_res = sin_cos_calculate(theta);
	
	m_foc_unit.rotor_angle_sin_value_q15 = sin_cos_res.words.high;
	m_foc_unit.rotor_angle_cos_value_q15 = sin_cos_res.words.low;
	
	// 2. ����theta�жϵ�ǰ��������
	m_get_sector(theta);
	// 3. ����x y z����ֵ
	m_xyz_calculate();
	// 4. ����tcout��tbout��taout��ֵ������дGPT��ʱ��������ȽϼĴ�������ֵ������SVPWM����
	m_svpwm_duty_setting(Us);
	// 5. ����������ĵ�ѹ
	m_u_alpha_beta_calculate(Us);
}
