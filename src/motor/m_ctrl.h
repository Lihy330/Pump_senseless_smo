#ifndef __M_CTRL_H__
#define __M_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef enum
{
	MOTOR_STOP = 0,
	MOTOR_START,
	MOTOR_BOOST_CHARGING,
	MOTOR_ROTOR_PREPOS_START1,
	MOTOR_ROTOR_PREPOS_END1,
	MOTOR_ROTOR_PREPOS_START2,
	MOTOR_ROTOR_PREPOS_END2,
	MOTOR_DRAG,				// ���ǿ��״̬
	MOTOR_EXECUTE,
	MOTOR_TEST,
}m_motor_execute_state_machine_e;


typedef struct
{
	m_motor_execute_state_machine_e m_motor_execute_state_machine;
	uint8_t direction;
	volatile bool current_error_tick_flag;		// �ñ�־λ�����ж��Ƿ��Ѿ�����˵�����̬���Ĳ���
	volatile uint16_t speed;					// �����ת��
	volatile uint16_t last_speed;				// ��һ��ִ���ٶȻ�ʱ���ٶȴ�С
	volatile bool speed_update_flag;			// ת���Ƿ���µı�־
	volatile bool speed_cal_stable_flag;		// ת�ټ����ȶ��ı�־
	volatile int16_t iq_start_value;			// q����������
	volatile uint8_t speed_cal_stable_cnt;		// ת���ȶ��ļ�������
	
}m_motor_ctrl_t;

extern m_motor_ctrl_t m_motor_ctrl;


void motor_start(void);
void motor_stop(void);
void motor_boost_charging(void);
void motor_execute(void);


#ifdef _cplusplus
}
#endif

#endif