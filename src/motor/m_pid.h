#ifndef __M_PID_H__
#define __M_PID_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"
#include "m_parameter.h"

typedef struct
{
	uint16_t	kp;
	uint16_t 	ki;
	uint16_t 	kd;
	
	int16_t		kp_value;
	int16_t		ki_value;
	int16_t 	kd_value;
	
	int16_t		error;
	int16_t		last_error;
	
	int16_t		PID_output_value;
	
	int16_t 	PID_output_limit_value;
	int16_t 	ki_value_limit_value;
	
	int16_t		PID_target_value;
	int16_t		PID_actual_value;
	
	union_s32 	s_i_limit;		//积分项限幅值
	union_s32 	i_sum;			//积分项累加和 
	
}m_pid_unit_t;

extern m_pid_unit_t m_id_pid_unit;
extern m_pid_unit_t m_iq_pid_unit;
extern m_pid_unit_t m_spd_pid_unit;


void id_iq_pid_para_init();
void spd_pid_para_init();
int16_t parallel_position_pid_func(m_pid_unit_t *pid);
int16_t series_pid_func(m_pid_unit_t *pid);
int16_t chenggong_series_pid_algorithm(m_pid_unit_t *pid);
int16_t chenggong_parallel_position_pid_algorithm(m_pid_unit_t *pid);



#ifdef _cplusplus
}
#endif

#endif