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


// �������������˼·
// ADC�ɼ�ֵ��Χ 0-4095  ����ֵ�ѹ * 10�� + 1.65V + ��̬���
// �����˷ŵľ�̬����ADCֵΪ100����ô�����ڵ����ת֮ǰ�ɼ�����ADCֵΪ�˷ŵľ�̬���Ŵ�10��֮���ADCֵ��100��������1.65V��ADCֵ2048��Ҳ����2148
// �����ת֮�󣬲ɼ���������ֵ�ѹ * 10�� + 1.65V + ��̬��� * 10����ADCֵ��������1148�����Ǽ���2148���õ�-1000����ô���ʱ��õ��ľ��ǲ�ֵ�ѹ�Ŵ�10��֮���ֵ
// ��ˣ����Ǽ�����̬���֮����ֵ��Χ����� -2048 -- 2048
// ������Ǽ�����̬֮��Ϊ0����ô˵����ǰ�Ĳ�ֵ�ѹΪ0�����������̬���֮��Ϊ-2048��˵����ǰ��ֵ�ѹΪ-165mV
// ��ˣ�-2048��Ӧ��ֵ�ѹΪ-1.65V��2048��Ӧ��ֵ�ѹΪ1.65V
// ��ˣ����ǽ�������̬���Ŵ�10�����1.65V��ADCֵ֮�����Ǿ͵õ��˵����Ĳ�ֵ�ѹ��ADCֵ�����ֵ�ķ�Χ��-2048 -- 2048���൱����Q12��ʽ�����ݣ�-4096 -- 4096��
// ��������3λ��ΪQ15��ʽ�����ݣ�-32768 -- 32768��,����ʵ����ֵ�ķ�Χ�� -16384 -- 16384��Ҳ����-1.65V -- 1.65V
// ��ʵ���ǲɼ������ǵ�ѹֵ�����ǿ��Դ������ֵ����Ϊ����ֻ���һ������������ֵ��֮һ����������ֱ���òɼ����ĵ�ѹֵ���ɽ��к����㷨��ʵ��


m_current_cal_unit_t m_current_cal_unit;


// �ú�������������һ�������ת֮ǰ�ɼ���̬����ʱ�䣬ʱ�����0֮��ADC�жϴ�ֹͣ�Ծ�̬���Ĳɼ��ͼ��㣬Ȼ��ʼ������������ִ��
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

// ���ɼ���������ʵʱ����ֵ����ȥ��̬���õ���ʵ�Ĳ����������˵ĵ���ֵ��ע�⣬ʵ�����ǲ����������˵ĵ�ѹֵ�������ֵ����һ����������ֵ��֮һ�ı�����ϵ��
void get_tripple_phase_current()
{
	int16_t ia = 0;
	int16_t ib = 0;
	int16_t ic = 0;
	
	// ����Ӧ������ʵʱ��⵽�ĵ���ֵ��ȥ��̬���ֵ���������������Ŵ���������ֱ���þ�̬�����Ϊ���������Ͳ��ó���-1����������ֵ�ķ�����
	ia = (int16_t)((adc_unit.ia_static_error - adc_unit.u_current.instant_value) << 3);  // ��ֵ��-2048 -- 2048 Ȼ������3λ�����Q15��ʽ
	ib = (int16_t)((adc_unit.ib_static_error - adc_unit.v_current.instant_value) << 3);
	ic = (int16_t)((adc_unit.ic_static_error - adc_unit.w_current.instant_value) << 3);
	
	
		/*����������ţ���������С�ĵ�������KCL���ɼ���*/
	switch(m_svpwm_unit.sector)
	{
		case 1:
			ia = 0 - ib - ic;	//U�����������С,KCL���ɼ����������С����
		break;
		case 2:
			ib = 0 - ia - ic;	//V�����������С,KCL���ɼ����������С����
		break;
		case 3:
			ib = 0 - ia - ic;	//V�����������С,KCL���ɼ����������С����
		break;
		case 4:
			ic = 0 - ia - ib;	//W�����������С,KCL���ɼ����������С����
		break;
		case 5:
			ic = 0 - ia - ib;	//W�����������С,KCL���ɼ����������С����
		break;
		case 6:
			ia = 0 - ib - ic;	//U�����������С,KCL���ɼ����������С����
		break;
		default:
		break;
	}
	
	
	m_current_cal_unit.q15_ia = ia;
	m_current_cal_unit.q15_ib = ib;
	m_current_cal_unit.q15_ic = ic;
}

// Clark�任
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
	
	// ȡQ31�ĸ�λ
	m_current_cal_unit.q15_i_alpha = i_alpha;
	m_current_cal_unit.q15_i_beta = i_beta.words.high;
}

// Park�任
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


// Usģ�������Լ���ǰ�Ǽ���
void calculate_Us_thetaC_func()
{
	uint16_t Us2;
	uint16_t Ud2;
	uint16_t Uq2_max;
	uint16_t Uq2;
	float Uq_max_f;
	float Us_f;
	float theta_c_rad_f;		// dsp������Ļ��ȵĳ�ǰ��
	float theta_c_ang_f;
	int16_t Uq_max;
	
	/*******************************************************Usģ������*******************************************************************/
	// ͨ�����ٶȻ������Uq�����޷����Ϳ���ʵ�ֶ�Us���޷�
	// ʵ����UsӦ����һ��Q16��ʽ�����ݣ���������Ҫͨ��Us^2 - Ud^2�õ�Uq^2����Uq��һ��Q15�����ݣ������з���ģ����Ի������и���������Ϊ�˷������
	// ���ǽ�Us_max����޷�ֵ����Ϊ��Q15��ʽ
	m_current_cal_unit.Us_max = Q15_US_MAX_VALUE;
	Us2 = (uint16_t)((uint32_t)(m_current_cal_unit.Us_max * m_current_cal_unit.Us_max) >> 14);	// Q15 * Q15 => Q30; Q30 >> 14 => Q16
	Ud2 = (uint16_t)((uint32_t)(m_current_cal_unit.q15_Ud * m_current_cal_unit.q15_Ud) >> 14);	// Q15 * Q15 => Q30; Q30 >> 14 => Q16
	
	// ����Uq������޷�ֵ��ƽ��
	Uq2_max = Us2 - Ud2;
	
	// �Ƚ�Uq2_maxת���ɶ�Ӧ�ĸ�������Ȼ���Uq2_max���п�������
	arm_sqrt_f32((float)Uq2_max / 65536.0f, &Uq_max_f);
	Uq_max = (int16_t)((uint16_t)(Uq_max_f * 65536.0f) >> 1);
	
	// ��Uq�����޷�
	if (m_current_cal_unit.q15_Uq > Uq_max)
	{
		m_current_cal_unit.q15_Uq = Uq_max;
	}
	else if (m_current_cal_unit.q15_Uq < -Uq_max)
	{
		m_current_cal_unit.q15_Uq = -Uq_max;
	}
	
	// ����Us
	Uq2 = (uint16_t)((uint32_t)(m_current_cal_unit.q15_Uq * m_current_cal_unit.q15_Uq) >> 14);	// Q15 * Q15 => Q30; Q30 >> 14 => Q16
	Us2 = Uq2 + Ud2;
	
	// �Ƚ�Us2ת���ɶ�Ӧ�ĸ�������Ȼ���Us2���п�������
	arm_sqrt_f32((float)(Us2 / 65536.0f), &Us_f);
	// ��������ת����Q16��ʽ������
	m_foc_unit.Us = (uint16_t)(Us_f * 65536.0f);
	
	/*******************************************************��ǰ�Ǽ���*******************************************************************/
	// �ж�Uq��Ud��ֵ
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
				// ��ʱ��ʱ��Ud�Ǹ�ֵ������������Ҫ���㳬ǰ�ǵľ���ֵ��������Ҫ��Ud����-1�任Ϊ��ֵ
				arm_atan2_f32((float)((-1) * m_current_cal_unit.q15_Ud / 32768.0f), (float)(m_current_cal_unit.q15_Uq / 32768.0f), &theta_c_rad_f);
				// ��������תΪ�Ƕ���
				theta_c_ang_f = theta_c_rad_f / PI * 180.0f;
				// �Ƕȹ�һ��
				theta_c_ang_f = theta_c_ang_f / 360.0f;
				m_foc_unit.theta_c = (uint16_t)(theta_c_ang_f * 65536.0f);
			break;
			case CW:
				arm_atan2_f32((float)(m_current_cal_unit.q15_Ud / 32768.0f), (float)(m_current_cal_unit.q15_Uq / 32768.0f), &theta_c_rad_f);
				// ��������תΪ�Ƕ���
				theta_c_ang_f = theta_c_rad_f / PI * 180.0f;
				// �Ƕȹ�һ��
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
	/*Usģ������޷���90%*/
	m_foc_unit.q15_us_max = US_MAX_VALUE;
	
	/*
		Ud^2+Uq^2=Us^2
		Q15*Q15=Q30,Q30����14λ=Q16
	*/
	ud_2 = (uint16_t)((uint32_t)(m_foc_unit.coordinate.q15_ud * m_foc_unit.coordinate.q15_ud) >> 14);
	uq_2 = (uint16_t)((uint32_t)(m_foc_unit.coordinate.q15_uq * m_foc_unit.coordinate.q15_uq) >> 14);
	us_max_2 = (uint16_t)((uint32_t)(m_foc_unit.q15_us_max * m_foc_unit.q15_us_max) >> 14);
	
	/*���Ud^2+Uq^2<=Usģ�����ֵ��ƽ������������*/
	if((ud_2 + uq_2) <= us_max_2)
	{
		us_2 = ud_2 + uq_2;
	}
	/*Usģ������޷�*/
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
	
	/*Usģ������޷���90%*/
	m_current_cal_unit.Us_max = Q15_US_MAX_VALUE;
	
	/*
		Ud^2+Uq^2=Us^2
		Q15*Q15=Q30,Q30����14λ=Q16
	*/
	ud_2 = (uint16_t)((uint32_t)(m_current_cal_unit.q15_Ud * m_current_cal_unit.q15_Ud) >> 14);
	us_max_2 = (uint16_t)((uint32_t)(m_current_cal_unit.Us_max * m_current_cal_unit.Us_max) >> 14);
	/*
		ʵ��ʵ���ϣ���������ͶӰ�ķ�������ΪUd���ڷ�Χ�ڣ�����
		��������Uq�����ֵ����ô�͸���uq_2 = us_max_2 - ud_2;�����ʽ�����
		uq_max_2���Uq���ֵ��ƽ��
	*/
	uq_max_2 = us_max_2 - ud_2;
	
	/*
		(float)uq_2 / 65536.0f����Q16��ʽ������ת��Ϊ��Ӧ��С��
		uq_max_f������������Ӧ��Q16��ʽ��С��
	*/
	arm_sqrt_f32((float)uq_max_2 / 65536.0f, &uq_max_f);
	/*
		uq_max_f * 65536.0f:Q16��ʽ��Uq���ֵ
		((uint16_t)(uq_max_f * 65536.0f)) >> 1������1λת��ΪQ15��ʽ������
		��Ϊuq_max��Ҫ��Uq���Q15��ʽ���������Ƚ�
	*/
	uq_max = ((uint16_t)(uq_max_f * 65536.0f)) >> 1;
	
	/*Uq�������ֵ�޷�*/
	if(m_current_cal_unit.q15_Uq > uq_max)
	{
		m_current_cal_unit.q15_Uq = uq_max;
	}
	/*Uq������Сֵ�޷�*/
	if(m_current_cal_unit.q15_Uq < -uq_max)
	{
		m_current_cal_unit.q15_Uq = -uq_max;
	}
	
	/*
		Ud^2+Uq^2=Us^2
		Q15*Q15=Q30,Q30����14λ=Q16
	*/
	uq_2 = (uint16_t)((uint32_t)(m_current_cal_unit.q15_Uq * m_current_cal_unit.q15_Uq) >> 14);
	us_2 = ud_2 + uq_2;
#endif
	
	/*
		arm dsp��api�ӿڽ��п�������
		��Q16��ʽ������ת��ΪС����ʽ�ı��
		Us�������͵����ݼ������ֵΪ0.9
	*/
	arm_sqrt_f32((float)us_2 / 65536.0f, &us_f);
	/*����Usģ����ת��ΪQ16��ʽ*/
	m_foc_unit.Us = (uint16_t)(us_f * 65536.0f);
	
	/*��ǰ�Ǽ��㣺arctan(Ud/Uq)*/
	/*���Uq=0��Ud/Uq���������򲻳���*/
	if(m_current_cal_unit.q15_Uq == 0)
	{
		m_foc_unit.theta_c = 0;//��ǰ��ֱ������Ϊ0��
	}
	/*���Udֵ����0��˵��Us��ȫ����q���ϣ���ǰ��Ϊ0��*/
	else if(m_current_cal_unit.q15_Ud == 0)
	{
		m_foc_unit.theta_c = 0;//��ǰ��ֱ������Ϊ0��
	}
	else
	{
		float x,y;
		y = (float)m_current_cal_unit.q15_Ud / 32768.0f;
		x = (float)m_current_cal_unit.q15_Uq / 32768.0f;
		float result_rad_f;
		float result_angle_f;
		
		#define PI_DIV_180			(180.0f / M_PI)
		#define RAD_TO_ANGLE(rad)	(float)(rad * PI_DIV_180)  //�Ƕ�ֵ = ����ֵ �� (180 / ��)   
		
		
		switch(m_motor_ctrl.direction)
		{
			case CCW:
				/*
					Returned value is between -Pi and Pi
					��ʱ����תUqΪ����UdΪ��
					��Ϊ��Ud��Uq������Ϊ����ֵ����������0~��֮��(0��-180��)
					ʵ�ʼ�����Ӧ����0~��/2֮�䣬Ҳ����0��-90��֮��
				*/						
				arm_atan2_f32(-y, x, &result_rad_f);
				/*����ת�Ƕ�*/
				result_angle_f = RAD_TO_ANGLE(result_rad_f);
				// �Ƕȹ�һ��
				result_angle_f = result_angle_f / 360.0f;
				m_foc_unit.theta_c = (uint16_t)(result_angle_f * 65536.0f); //����ǰ��ת��ΪQ16��ʽ
			break;
			case CW:
				/*
					Returned value is between -Pi and Pi
					˳ʱ����תUqΪ����UdΪ��
					��Ϊ��Ud��Uq������Ϊ����ֵ����������0~��֮��(0��-180��)
					ʵ�ʼ�����Ӧ����0~��/2֮�䣬Ҳ����0��-90��֮��
				*/						
				arm_atan2_f32(-y, -x, &result_rad_f);
				/*����ת�Ƕ�*/
				result_angle_f = RAD_TO_ANGLE(result_rad_f);
				// �Ƕȹ�һ��
				result_angle_f = result_angle_f / 360.0f;
				m_foc_unit.theta_c = (uint16_t)(result_angle_f * 65536.0f); //����ǰ��ת��ΪQ16��ʽ
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


