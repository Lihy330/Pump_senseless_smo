#include <stdio.h>
#include "m_rotor_angle.h"
#include "m_parameter.h"
#include "../drv/drv_hall.h"
#include "../drv/drv_hall_cb.h"
#include "m_ctrl.h"
#include "../math/math_utils.h"
#include "m_foc.h"
#include "m_observer.h"


// һ��ע�⣬�����Ƕ������ֵ�Ķ�Ӧ��ϵ��������Ҫ��ģ�����������û��ѧ
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
	uint8_t start_cnt;		// ���������ʼ�׶�ͳ�ƻ����źŸ��´���
	uint16_t update_cnt;	// ����δ���µĴ�����ʱ�䣩���������趨����ֵ���쳣
	bool start_sign;
	bool init_angle_sign;	// �Ƿ��ǳ�ʼλ�ý�
	bool update_sign;
}m_hall_unit_t;

m_hall_unit_t m_hall_unit;

void m_hall_get_value()
{
	uint8_t u_val, v_val, w_val;
	// ��ȡ����������
	R_IOPORT_PinRead(g_ioport.p_ctrl, HALL_U_IO_PORT, &m_hall_unit.u_value);
	R_IOPORT_PinRead(g_ioport.p_ctrl, HALL_V_IO_PORT, &m_hall_unit.v_value);
	R_IOPORT_PinRead(g_ioport.p_ctrl, HALL_W_IO_PORT, &m_hall_unit.w_value);

	u_val = (uint8_t)m_hall_unit.u_value;
	v_val = (uint8_t)m_hall_unit.v_value;
	w_val = (uint8_t)m_hall_unit.w_value;

	// �����������������ֵ��ϣӳ��
	m_hall_unit.hall_value = (uint8_t)((u_val << 2) | (v_val << 1) | w_val);
	// �жϵ�ǰ����ֵ�Ƿ����
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
	
	// ����hall����ֵ
//	drv_hall_capture.hall_reset_func();
	drv_hall_capture.hall_uvw_sign_clear_func();
	// ��ȡת�ӳ�ʼλ�û���ֵ
	m_hall_get_value();
	rotor_angle_inc.u32 = 0;
	// ��ȡת�ӳ�ʼλ�ý�
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
		
		/************************************** ���ڵ������ת����ȷ����ǰת��λ�ý� *********************************************/
		switch(m_motor_ctrl.direction)
		{
			case CW:
				rotor_angle.u32 = ROTOR_CW_ANGLE[m_hall_unit.hall_value];
			break;
			case CCW:
				rotor_angle.u32 = ROTOR_CCW_ANGLE[m_hall_unit.hall_value];
			break;
		}
		
		/************************************** ���ݻ����ź����䣬����60��Ƕ�ִ��ʱ�� *********************************************/
		
		if (drv_hall_capture.u_sign || drv_hall_capture.v_sign || drv_hall_capture.w_sign)
		{
			// ����u����
			if (drv_hall_capture.u_sign)
			{
				m_hall_unit.angle_temp_time = drv_hall_capture.u_capture_value;
			}
			// ����v����
			else if (drv_hall_capture.v_sign)
			{
				m_hall_unit.angle_temp_time = drv_hall_capture.v_capture_value;
			}
			// ����w����
			else if (drv_hall_capture.w_sign)
			{
				
				m_hall_unit.angle_temp_time = drv_hall_capture.w_capture_value;
			}
			// ����������
			drv_hall_capture.hall_uvw_sign_clear_func();
		}
		
		/*60���Ƕ�ʱ�����*/
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
		
		// �ȶ���ȡ��10�λ����źŸ���֮��������Ϊת�ټ����ȶ�
		if (m_motor_ctrl.speed_cal_stable_cnt ++  >= 12)
		{
			m_motor_ctrl.speed_cal_stable_cnt = 12;
			m_motor_ctrl.speed_cal_stable_flag = true;
			m_motor_ctrl.speed_update_flag = true;
		}
		// ��ȡ��60���Ƕȵ�ʱ��
		m_hall_unit.angle_60_time = m_hall_unit.angle_60_time_filter2;
		
		
		/************************************** ����60���Ƕ�ʱ�����Ħ� *********************************************/
		rotor_angle_inc.u32 = (uint32_t)((60.0f * 65536.0f * 50.0f) / (360.0f * (float)m_hall_unit.angle_60_time * 1.0f));
		// ��¼һ�µ�ǰת��
		m_motor_ctrl.speed = (uint16_t)(60000000 / (m_hall_unit.angle_60_time * 6 * POLAR));
	}
	else
	{
		if (m_hall_unit.update_cnt < HALL_VALUE_UPDATE_THRESHOLD_TIME)
		{
			m_hall_unit.update_cnt ++ ;
			// ���ݵ����ת����ȷ���Ƕȵĸ���
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
			// �쳣����
			m_hall_unit.update_cnt = 0;
		}
//		printf("rotor_angle.u32: %d\r\n", rotor_angle.u32);
	}
	return (uint16_t)rotor_angle.words.low;
}


/*
	�������ܣ���ʼ���޸нǶȼ������ز���
*/
void m_sensorless_angle_calculate_init()
{
	// delta_theta_e�ĵ�ͨ�˲���һ�㽫��ֹƵ������Ϊ300RPM
	/*
		RPM = 300 => RPS = 300 / 60 = 5;
		eRPS = fc = RPS * Polar = 10;
		���� fc = 10;
		
		�����ͨ�˲����˲�ϵ�� K = 2�� * Tpwm * fc
		K = 2 * 3.1415926f * (1 / 20000) * 10 = 2 * 3.1415926f * 0.00005 * 10 = 0.0031415926
		K = 2 * 3.1415926f * (1 / 20000 * 20) * 10 = 2 * 3.1415926f * 0.00005 * 10 = 0.062831852 	?
		ת����Q15��ʽ
		K = 0.0031415926 * 32768 = 103
		K = 0.062831852 * 32768 = 2060	?
	*/
	m_obs_angle_unit.q15_delta_theta_e_filter_k = DELTA_ANGLE_FILTER_K;
}


/*
	�������ܣ����㷴�綯�������λ�üн�theta_e��q15��ʽ
	����ֵ��δ������theta_e�����ں����Ľ��ٶȼ���
	�����ĽǶ�ֵ�洢��m_foc_unit�ṹ����
*/
void m_sensorless_theta_e_calculate()
{
	int16_t _q15_theta_e_uncompensated;
	
	_q15_theta_e_uncompensated = m_arctan_calculate(m_obs_unit.q15_e_alpha_n_estimation_final, m_obs_unit.q15_e_beta_n_estimation_final);
	
	// ���ڵ�ǰ����ת���������λ�����������ڷ��綯�ƻ�ȡ�����н��������ε�ͨ�˲�����ֹƵ���뷴�綯��Ƶ����ȣ�����λ�ͺ���90�㣬�ڴ˴�����
	switch(m_motor_ctrl.direction)
	{
		case CCW:
			/* ����һ������Ϊʲôq15��q16�Ƕ�ֵ�ܹ�ֱ����Ӽ���
				�����м��㺯�����ص���Ȼ��q15��ʽ�ĽǶ�ֵ�����ǹ�һ���Ļ�׼�ǦУ����Դ���ȷ�������ʵ�ʽǶ�ֵΪ90��
					��ô��һ��֮����0.5��Ȼ�����32768���õ�����16384��
				��ANGLE_90��һ���Ļ�׼��2�У���ˣ�90���һ��֮��Ϊ0.25��Ȼ�����65536�õ�����Ȼ��16384
				���Զ���������ĽǶ�ֵ��ͳһ�ģ���˿���ֱ����Ӽ�
				���ң�Ϊʲô�������������������δ������theta_eΪ180�㣬��ô��һ��֮���ֵΪ32768����ʱ����ANGLE_90
					֮��ͻᳬ��q15�����ֵ���������֮��������´�q15����Сֵ-32767��ʼ������������֮����Ȼ��q15����Ȼ��-180��~180��
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
	�������ܣ���������еת��
	����ֵ��q15��ʽ�Ļ�еת��
	���ٶȼ���Ĳɼ�������Tangle һ������Ϊ1ms
*/
void m_sensorless_rpm_calculate()
{
	int16_t erps_temp = 0;
	
	switch(m_motor_ctrl.direction)
	{
		case CCW:
			// ��ÿ��pwm���ڣ�50us��������ĽǶȱ仯�����
			// ���������һ�εĽǶ�ֵ��170�㣬��ǰ�Ƕ���-170�㣬��ô�����ϲ�ֵΪ20�㣬��ʵ��Ҳ����ôִ�е�
			// -170���ȥ10���Ϊ-180�㣬Ȼ���ټ�ȥ160�㣬��ô�ʹ�180����20�㣬�պ��ǲ�ֵ���������ֻҪ���ǵĲ�ֵ��Ȼ������int16_t�У�C���ԾͿ����Զ���������Ĵ���
			m_obs_angle_unit.q15_delta_angle_Tangle_sum += m_obs_angle_unit.q15_theta_e_uncompensated - m_obs_angle_unit.q15_theta_e_uncompensated_last;
		break;
		case CW:
			m_obs_angle_unit.q15_delta_angle_Tangle_sum += m_obs_angle_unit.q15_theta_e_uncompensated_last - m_obs_angle_unit.q15_theta_e_uncompensated;
		break;
	}
	// ����ǰ��q15_theta_e_uncompensated_last��q15_theta_e_uncompensated����
	m_obs_angle_unit.q15_theta_e_uncompensated_last = m_obs_angle_unit.q15_theta_e_uncompensated;
	
	m_obs_angle_unit.delta_angle_Tangle_cnt ++ ;
	
	// �����ǰ�ۼ��ۼ���һ���ɼ����ڣ���ô�ͶԵ�ǰ�� q15_delta_angle_Tangle_sum ��ֵ�� q15_delta_angle_Tangle
	if (m_obs_angle_unit.delta_angle_Tangle_cnt >= DELTA_ANGLE_CNT)
	{	
		m_obs_angle_unit.q15_delta_angle_Tangle = m_obs_angle_unit.q15_delta_angle_Tangle_sum;
		
		m_obs_angle_unit.q15_delta_angle_Tangle_sum = 0;
		m_obs_angle_unit.delta_angle_Tangle_cnt = 0;
	}
	
	// Ϊ�˺����Ƕȼ����ƽ������Ҫ�Ի�ȡ����delta_angle���е�ͨ�˲������ý�ֹƵ��Ϊ300RPM
	m_obs_angle_unit.q15_delta_angle_Tangle_filtered = m_obs_digital_LPF(m_obs_angle_unit.q15_delta_theta_e_filter_k, m_obs_angle_unit.q15_delta_angle_Tangle, m_obs_angle_unit.q15_delta_angle_Tangle_filtered);
	
	// �˲���Ϻ����ת��
	// ��ǰ�Ƕ�ֵ�ĺ��壺����Q��ʽ��������˵������һ����һ����ֵ������Q15����ֵ���֣�0~32768����ʾ�ľ��ǣ�0~1��
		// ����ȷ�����ǰ��Q15��ֵ��19660����Ӧ����0.6���ͱ�ʾ��ǰ���ֵ��0.6���Ĺ�һ����׼ֵ��Ҳ����0.6����
		// 		��ͨ��һ�㽲������0.6����Ȧ��Ȼ��������������������������1��Tangle���ڣ�������0.001s����1ms��
		// 		ת����ת��0.6����Ȧ������0.001s����ôҲ����˵��ת����1s����ת��0.6 / 0.001 = 600����Ȧ��Ҳ����300Ȧ
		// 		Ҳ����˵ת��rps = (q15_delta_angle_Tangle / Tangle) / 32768 / 2������32768 / 2���൱��>>16
		// 		Ҳ����˵rps = (q15_delta_angle_Tangle / Tangle) >> 16
		// ���ǾͰ�Q��ʽ�����ݿ�����һ����ֵ�ͻ�ܺ�����ˣ�ֻ�ǲ�ͬ�ı������в�ͬ�Ĺ�һ����׼ֵ
	
	
//	erps_temp = (int16_t)((int32_t)(m_obs_angle_unit.q15_delta_angle_Tangle_filtered * DELTA_ANGLE_TIME_INVERSE) >> 16);
//	// ��ת���ٴν��в��岨���˲�
//	m_obs_angle_unit.erps_filter1 = LPF_CALC(erps_temp, m_obs_angle_unit.erps_filter1);
//	m_obs_angle_unit.erps_filter2 = LPF_CALC(m_obs_angle_unit.erps_filter1, m_obs_angle_unit.erps_filter2);
//	m_obs_angle_unit.erps = LPF_CALC(m_obs_angle_unit.erps_filter2, m_obs_angle_unit.erps);
//	
//	m_obs_angle_unit.erpm = m_obs_angle_unit.erps * 60;
//	m_obs_angle_unit.rps = m_obs_angle_unit.erps / POLAR;
//	m_obs_angle_unit.rpm = m_obs_angle_unit.rps * 60;
	
	// ����ط�һ��Ҫ����60000�����ܳ���1000��Ȼ������ٳ���60����Ϊֻ����1000���õ�����ֵ���ܻ�С��65536������֮�����0
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
	�������ܣ�һ�׵�ͨ�˲�������Ӧ�˲�ϵ������

	K = 2�� * Tpwm * fc
	fc = (�Ħ�/360) / ��T
	K = �� * Tpwm * ((�Ħ�/180) / ��T) = ((�� * Tpwm) / ��T) * (�Ħ�/180)
	K = ((�� * Tpwm) / ��T) * q15_delta_angle_Tangle_filtered
	��((�� * Tpwm) / ��T)����һ��ϵ��Q16_LPF_SUB_K���任Ϊq16������֮����q31��������16λ�͵õ���q15������Ӧ�˲�ϵ��
	�������Ƶ��Ĺ�ʽ����(�Ħ�/180)�������Ǿ���arctan�����м���õ���q15�Ƕ�ֵ�ı仯�����Ѿ��ڼ�����ٶȵĺ����л�ȡ����

	300RPM��Ӧ�ĦĦȼ��� �� MIN_FC_DELTA_THETA
	
*/
void m_sensorless_LPF_K_calculate()
{
	// �˲�֮��ĽǶ������Ħ�
	int16_t val = m_obs_angle_unit.q15_delta_angle_Tangle_filtered;
	
	// �жϵ�ǰ�ĽǶ������Ƿ������ֵ�������ֵ�����ת��300RPM����Ӧ�ĦĦȣ����ڦĦ�������Ȼ����300RPMת���µ��˲�ϵ��
	/*
		RPM = 300 => RPS = 300 / 60 = 5;
		eRPS = fc = RPS * Polar = 10;
		���� fc = 10;
		
		fc = (�Ħ�/360) / ��T������fc=10
		fc * ��T = (�Ħ�/ 180) / 2
		fc * ��T * 2 = (�Ħ�/ 180) = 10 * 0.001 * 2 = 0.02
		(�Ħ�/ 180) = 0.02
		ת����q15����0.02 * 32768 = 655
		
		��Ҫע����ǣ�����ΪʲôҪ�ճ�(�Ħ�/ 180) ����Ϊ����ͨ�������м�����ĽǶȣ���һ���Ļ�׼����180�㣨�У�
		�����Ǿ仰��q��ʽ���ݾ���һ����һ�������֣�0.02��ʾʲô�أ���ʾ�ľ���0.02����Ȧ�ĽǶ�ֵ
	*/
	if (val < MIN_FC_DELTA_THETA)
	{
		val = MIN_FC_DELTA_THETA;
	}
	int32_t k_val = (int32_t)Q16_LPF_SUB_K * val;
	m_obs_unit.q15_klpf = (int16_t)(k_val >> 16);
}

/*
	�������ܣ�ִ���޸еķ��綯�������λ�ýǼ���Ĵ���
	����ֵ��q15_theta_e_compensated ����֮���theta_e
*/
int16_t m_sensorless_theta_e_execute()
{
	// ͨ�������м���theta_e����������λ
	m_sensorless_theta_e_calculate();
	// ����δ������theta_e����ת�٣����Ҷ�delta_theta_e�����˲�
	m_sensorless_rpm_calculate();
	// �����˲����delta_theta_e����һ�׵�ͨ�˲���������Ӧ�˲�ϵ��
	m_sensorless_LPF_K_calculate();
	return m_obs_angle_unit.q15_theta_e_compensated;
}


