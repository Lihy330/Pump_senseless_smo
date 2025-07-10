#ifndef __M_TICK_H__
#define __M_TICK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef struct
{
	uint8_t spd_tick_cnt;
	uint8_t boost_charging_tick_cnt;
	uint16_t rotor_prepos1_tick_cnt;		// ��һ��ת��Ԥ��λ��ʱ��
	uint16_t rotor_prepos2_tick_cnt;		// �ڶ���ת��Ԥ��λ��ʱ��
	uint16_t drag_tick_cnt;					// ����ǿ�����ڼ���ֵ
	uint16_t iq_calib_tick_cnt;				// ����Iq������˥�������ڼ�������
	uint16_t spd_loop_tick_cnt;				// �ٶȻ�ִ������
	uint16_t current_static_error_tick_cnt;
	uint16_t test_current_tick_cnt;			// �����������ÿ��50ms����ӡһ�ε�ǰ�ɼ������������ֵ������ʱʹ��
	uint16_t speed_set_value_tick_cnt;		// �����������б�¼��������ٶ�Ŀ��ֵ�ĸ���ʱ��	200ms
	
}m_tick_unit_t;

extern m_tick_unit_t m_tick_unit;

void tick_execute(void);


#ifdef _cplusplus
}
#endif

#endif