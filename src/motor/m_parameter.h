#ifndef __M_PARAMETER_H__
#define __M_PARAMETER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef union
{
	uint32_t u32;
	struct
	{
		uint16_t low;
		uint16_t high;
	}words;
}union_u32;


typedef union
{
	int32_t s32;
	struct
	{
		int16_t low;
		int16_t high;
	}words;
}union_s32;


#define		M_PI		3.14159265358f

#define     ANGLE_0     0
#define     ANGLE_30    5461
#define     ANGLE_60    10922
#define     ANGLE_90    16384
#define     ANGLE_120   21845
#define     ANGLE_150   27306
#define     ANGLE_180   32768
#define     ANGLE_210   38229
#define     ANGLE_240   43690
#define     ANGLE_270   49151
#define     ANGLE_300   54613
#define     ANGLE_330   60074
#define     ANGLE_360   0

// �����ת����
#define 	CW			0
#define		CCW			1


// Timer Clock Frequency
#define GPT_CLK									(120000000.0f)
// PWM Frequency
#define PWM_FREQUENCY							(20000.0f)
// Tpwm
#define T_PWM									(float)(1.0f / (float)PWM_FREQUENCY)
// PWM Period Count
#define PWM_PERIOD_VALUE						(uint32_t)(GPT_CLK / PWM_FREQUENCY)   // 6000
// Timer Reload Value	
#define GPT_RELOAD_VALUE						(PWM_PERIOD_VALUE >> 1)		// 3000


// ���������
#define 	POLAR								2


// 60���Ƕ�ʱ�����
// scale��ʾ����һ�ε�ʱ���Ƕ���us
#define HALL_TIME_CALCULATE(speed, scale) 		(uint32_t)(60000000.0f / (6.0f * (float)speed * 2.0f * (float)scale))
// ���Լ�
#define MAX_SPEED_HALL_TIME_VALUE					HALL_TIME_CALCULATE(3000, 1)
#define MIN_SPEED_HALL_TIME_VALUE					HALL_TIME_CALCULATE(50, 1)


// �˲�����
#define LPF_CALC(X_in, Y_out)						(X_in >> 2) + (Y_out >> 1) + (Y_out >> 2)

// Q����������
#define IQ_START_VALUE								2048



/**************************************�����������صĲ���*****************************************/

// Usģ������ʱ��
#define SPD_TICK_CNT								10			// 10 * 50us  => 0.5ms
// �Ծٵ���Ԥ���ʱ��
#define BOOST_CHARGING_TICK_CNT						200			// 200 * 50us => 10ms
// ����ֵδ����ʱ����ֵ
#define HALL_VALUE_UPDATE_THRESHOLD_TIME			(uint16_t)(1000.0f * 20.0f)
// ����˷ž�̬������ʱ��
#define CURRENT_STATIC_ERROR_TICK_CNT				1000 * 20 	// 20 * 1000 * 50us => 20 * 50ms => 1000ms => 1s
// ��ӡ�������������ʱ����
#define TEST_CURRENT_TICK_CNT						1000 * 10 	// 10 * 1000 * 50us => 500ms
// �ٶȻ�Ŀ��ת�ٸ���ʱ��
#define SPEED_SET_VALUE_TICK_CNT					20 * 20	// 20 * 20 * 50us => 20ms


/**************************************�޷���ز���*****************************************/

// ռ�ձ��޷�
#define DEAD_TIME									240		// 240/(120 * 10^6) => 2us
// ����������Сռ�ձ����෴�ĸ�����õ���Сռ�ձ���ʵ�����ռ�ձȣ�����ʵ�߼��ǻ���ģ�Ҳû����ϵ
#define MIN_DUTY_VALUE								DEAD_TIME
#define MAX_DUTY_VALUE								(GPT_RELOAD_VALUE - (DEAD_TIME >> 1) - DEAD_TIME)

// ��λ��ADC�ɼ��޷�ֵ  ������޷�ֵֻ��Ϊ�˵�λ����ת��һ��λ��ʱ�����ͣ����
#define MIN_US_TARGET_VALUE							655
#define MAX_US_TARGET_VALUE							58982

// ת���޷�ֵ����λ��rad/min��
#define MIN_SPEED_VALUE								1000
#define MAX_SPEED_VALUE								2600

// �ٶȻ������Q������޷�ֵ
#define IQ_MIN_VALUE								50

// Uq�޷�ֵ����Ӧ��Us��Q15�޷�ֵ
#define Q15_US_MAX_VALUE							29491	// 32768 * 0.9	��90%�����ֵ�޷�


// ������Q16��ʽ
// 2 / 3
#define _2DIV3										43691
#define SQRT3DIV2									56756
#define SQRT3DIV3									37837


#define DR_DIFF_CACULATE(us)					((uint32_t)(50.0f * 60.0f * 65536.0f / ((float)us * 360.0f)))
#define DR_DIFF_VALUE							DR_DIFF_CACULATE(1)	


/**********************************************************************�޸в��ֺ궨�����******************************************************************/

/**************************************�����������صĲ���*****************************************/
// �Ƕȱ仯����������ڼ���
#define	DELTA_ANGLE_CNT								20	
// T_ANGLE
#define	T_ANGLE										(float)(DELTA_ANGLE_CNT * T_PWM)	// 50us * 20 -> 1ms	
// 1�����ٶȼ������ڵ�ʱ��ĵ���
#define DELTA_ANGLE_TIME_INVERSE					(uint16_t)(1.0f / (float)T_ANGLE)


/**************************************��delta_theta_e��ͨ�˲���صĲ���*****************************************/
// delta_theta_e ��ͨ�˲��Ľ�ֹƵ��
// 300RPM����Ӧ�Ľ�ֹƵ��
#define DELTA_ANGLE_FILTER_FC						(float)(300.0f / 60.0f * (float)POLAR)
// delta_theta_e ��ͨ�˲����˲�ϵ�� 2�� * Tpwm * fc
#define DELTA_ANGLE_FILTER_K						(int16_t)((2.0f * M_PI * T_PWM * DELTA_ANGLE_FILTER_FC) * 32768.0f)
// �����Ƿ����delta_theta�ĵ�ͨ�˲����˲�ϵ������Ӧ�ò��ý��ٶȼ�������
#define DELTA_ANGLE_FILTER_K_1						(int16_t)((2.0f * M_PI * (T_PWM * (float)DELTA_ANGLE_CNT) * DELTA_ANGLE_FILTER_FC) * 32768.0f)

/**************************************�뷴�綯��һ�׵�ͨ�˲���صĲ���*****************************************/
// һ�׵�ͨ�˲���������Ӧ�˲�ϵ���ĸ���ϵ��((�� * Tpwm) / ��T) * 65536
#define Q16_LPF_SUB_K								(uint16_t)(((float)(M_PI * T_PWM) / T_ANGLE) * 65536.0f)
// ���ת��300RPM����Ӧ�ĦĦ�
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
#define MIN_FC_DELTA_THETA							(int16_t)(DELTA_ANGLE_FILTER_FC * T_ANGLE * 2.0f * 32768.0f)


/**************************************��������㷽����صĲ���*****************************************/
// ����ߵ���
#define LINE_RESISTANCE_U							13.2f		// ��
#define LINE_RESISTANCE_V							13.2f		// ��
#define LINE_RESISTANCE_W							13.3f		// ��
// ��������
#define PHASE_RESISTANCE							(float)((LINE_RESISTANCE_U + LINE_RESISTANCE_V + LINE_RESISTANCE_W) / 3.0f / 2.0f)	// ��

// ����ߵ��
#define LINE_INDUCTANCE_U							12.011f * 1000.0f	// uH
#define LINE_INDUCTANCE_V							13.352f * 1000.0f	// uH
#define LINE_INDUCTANCE_W							12.850f * 1000.0f	// uH
// �������
#define	PHASE_INDUCTANCE							(float)((LINE_INDUCTANCE_U + LINE_INDUCTANCE_V + LINE_INDUCTANCE_W) / 3.0f / 2.0f)	// uH

// �������Ʒ��̵�Fֵ
// F_COEFF = 1 - R * Tpwm / L
// T_PWM * 1000000 ��Ϊ�˽�0.00005s ת���� 50us
// �� * us / uH => �� * (1 / ��) => �����٣����Ǹ�ϵ����������32768���q15��ʽ����
#define F_COEFF										(int16_t)((1.0f - PHASE_RESISTANCE * T_PWM * 1000000.0f / PHASE_INDUCTANCE) * 32768.0f)

// G_COEFF = Tpwm / L
#define G_COEFF										(int16_t)((T_PWM * 1000000.0f / PHASE_INDUCTANCE) * 32768.0f)

/**************************************�뻬Ĥ�۲�����صĲ���*****************************************/
// ��Ĥ�۲���SAT�����ı߽�㣨������ֵ��
#define OBS_Z_MAX									170
// ��Ĥ�۲�������
#define OBS_K_GAIN									30000


/**************************************��ת��Ԥ��λ��صĲ���*****************************************/
// ��һ��Ԥ��λ��Usģ����С	
#define ROTOR_PREPOS_US1							19660
#define ROTOR_PREPOS1_TICK_CNT						10000	// 50us * 10000 => 500ms

// �ڶ���Ԥ��λ��Usģ����С	
#define ROTOR_PREPOS_US2							19660
#define ROTOR_PREPOS2_TICK_CNT						10000	// 50us * 10000 => 500ms


/**************************************�뿪��ǿ�ϼ�����صĲ���*****************************************/
// �Ǽ��ٶȲ���ֵ
#define DRAG_ACC_STEP								5
// ����ǿ�����ڼ���ֵ
#define DRAG_TICK_CNT								10
// ����ǿ������
#define T_DRAG										(float)(T_PWM * (float)(DRAG_TICK_CNT))	// 500us
// �л����ջ���Ĥ���ƵĻ�еת��
#define DRAG_MECH_SPEED								500

// ���ٶȵ����ֵ�޷�
/*
	���ǿ���ǿ�ϵĽ��ٶȣ�Ҳ����ÿ��PWM���ڵ����ĽǶ�ֵ�������޴󣬳��������Եķ��綯��������Ӧ���л���
	�ջ���Ĥ���ƣ������Ҫ���ýǶȵ��������ֵ��ͨ�����������ת��500RPMʱ�����ɿ����л����ջ�
	���������Ҫ����500RPM��еת�ٶ�Ӧ�ĵ�ת���µ�һ������ǿ������(500us)�е�q16_delta_theta
	eRPM = POLAR * RPM
	eRPS = eRPM / 60
	delta_theta = eRPS * 360 * 0.0005
	q16_delta_theta = (delta_theta / 360) * 65536 = 546
	
*/
#define Q16_DRAG_W_MAX_VALUE						(uint16_t)(((((float)DRAG_MECH_SPEED * (float)POLAR / 60.0f) * 360.0f * T_DRAG) / 360.0f) * 65536.0f)
// �����л����ջ��ĽǶȲ�ֵ���������ļ�����ֵ
#define OPEN_TO_CLOSE_CNT_THRESHOLD					3
// ��������Iq˥�����ڼ���ֵ
#define IQ_CALIB_TICK_CNT							100		// 50us * 100 => 5ms


/**************************************���ٶȻ���صĲ���*****************************************/
// �ٶȻ�ִ�����ڼ�������
#define SPEED_LOOP_TICK_CNT							200		// 50us * 200 => 10ms



#ifdef _cplusplus
}
#endif

#endif