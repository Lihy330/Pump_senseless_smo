#include <stdio.h>
#include "drv_adc_cb.h"
#include "../motor/m_tick.h"
#include "../math/math_trigonometric.h"
#include "../motor/m_foc.h"
#include "../motor/m_parameter.h"
#include "../motor/m_ctrl.h"
#include "../motor/m_observer.h"
#include "../motor/m_rotor_angle.h"

static adc_channel_t adc_group0_channels_list[ADC_GROUP0_NUM] = {
		ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3, ADC_CHANNEL_17, ADC_CHANNEL_12, ADC_CHANNEL_13, ADC_CHANNEL_14, ADC_CHANNEL_15
};

static adc_channel_t adc_group1_channels_list[ADC_GROUP1_NUM] = {
		ADC_CHANNEL_8, ADC_CHANNEL_9, ADC_CHANNEL_18
};


adc_unit_t adc_unit;

static void adc_group0_sample_data()
{
	int i;
	for (i = 0; i < ADC_GROUP0_NUM; i ++ )
	{
		R_ADC_B_Read(g_adc0.p_ctrl, adc_group0_channels_list[i], &adc_unit.adc_group0_original_data[i]);
	}
	adc_unit.u_current.instant_value = adc_unit.adc_group0_original_data[0];
	adc_unit.v_current.instant_value = adc_unit.adc_group0_original_data[1];
	adc_unit.w_current.instant_value = adc_unit.adc_group0_original_data[2];
	adc_unit.bus_current.instant_value = adc_unit.adc_group0_original_data[3];
	adc_unit.bus_voltage.instant_value = adc_unit.adc_group0_original_data[4];
	adc_unit.u_voltage.instant_value = adc_unit.adc_group0_original_data[5];
	adc_unit.v_voltage.instant_value = adc_unit.adc_group0_original_data[6];
	adc_unit.w_voltage.instant_value = adc_unit.adc_group0_original_data[7];
}

static void adc_group1_sample_data()
{
	int i;
	for (i = 0; i < ADC_GROUP1_NUM; i ++ )
	{
		R_ADC_B_Read(g_adc0.p_ctrl, adc_group1_channels_list[i], &adc_unit.adc_group1_original_data[i]);
	}
	adc_unit.over_current_voltage.instant_value = adc_unit.adc_group1_original_data[0];
	adc_unit.ntc_voltage.instant_value = adc_unit.adc_group1_original_data[1];
	adc_unit.speed_voltage.instant_value = adc_unit.adc_group1_original_data[2];
}

void adc_group0_filter_data()
{
	if (m_motor_ctrl.current_error_tick_flag == false)
	{
		adc_unit.u_current.sum_value += adc_unit.u_current.instant_value;
		adc_unit.v_current.sum_value += adc_unit.v_current.instant_value;
		adc_unit.w_current.sum_value += adc_unit.w_current.instant_value;
		adc_unit.u_current.count_value ++ ;
		adc_unit.v_current.count_value ++ ;
		adc_unit.w_current.count_value ++ ;
		if (m_tick_unit.current_static_error_tick_cnt == 0)
		{
			adc_unit.ia_static_error = (uint16_t)(adc_unit.u_current.sum_value / adc_unit.u_current.count_value);
			adc_unit.ib_static_error = (uint16_t)(adc_unit.v_current.sum_value / adc_unit.v_current.count_value);
			adc_unit.ic_static_error = (uint16_t)(adc_unit.w_current.sum_value / adc_unit.w_current.count_value);
			adc_unit.u_current.sum_value = 0;
			adc_unit.v_current.sum_value = 0;
			adc_unit.w_current.sum_value = 0;
			adc_unit.u_current.count_value = 0;
			adc_unit.v_current.count_value = 0;
			adc_unit.w_current.count_value = 0;
			m_motor_ctrl.current_error_tick_flag = true;
		}
	}
}

static void adc_group1_filter_data()
{
	adc_unit.speed_voltage.sum_value += adc_unit.speed_voltage.instant_value;
	adc_unit.speed_voltage.count_value ++ ;
	if (adc_unit.speed_voltage.count_value >= 16)
	{
		adc_unit.speed_voltage.count_value = 0;
		adc_unit.speed_voltage.average_value = (uint16_t)(adc_unit.speed_voltage.sum_value >> 4);
		adc_unit.speed_voltage.filter_value = (uint16_t)LPF_CALC(adc_unit.speed_voltage.average_value, adc_unit.speed_voltage.filter_value);
		adc_unit.speed_voltage.sum_value = 0;
	}
}


void adc0_callback(adc_callback_args_t * p_args)
{
	if (p_args->group_mask == ADC_GROUP_MASK_0)
	{
		if (p_args->event == ADC_EVENT_SCAN_COMPLETE)
		{
			#if 1
				adc_group0_sample_data();
				adc_group0_filter_data();
				get_tripple_phase_current();
				clark_transform();
				park_transform(m_foc_unit.rotor_angle);
				// 电流环执行
				current_pid_execute();
			
				{
					// 滑膜观测器执行
					m_obs_execute();
					// theta_e计算，delta_theta_e滤波，转速计算，自适应滤波系数计算
					m_sensorless_theta_e_execute();
				}
			
				// foc算法执行
				m_foc_algorithm_execute();
				#if 0
					test_current_func();	// 测试采集电流值打印，每500ms打印一次
				#endif
				tick_execute();
//				printf("m_foc_unit.Us: %d\r\n", m_foc_unit.Us);
			#elif 0		// test io output
				static uint8_t test_io_1_level = 0;
				test_io_1_level = !test_io_1_level;
				R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_12, test_io_1_level);
			#endif
		}
	}
	if (p_args->group_mask == ADC_GROUP_MASK_1)
	{
		if (p_args->event == ADC_EVENT_SCAN_COMPLETE)
		{
			#if 1
				adc_group1_sample_data();
				adc_group1_filter_data();
			#elif 0		// test io output
				static uint8_t test_io_2_level = 0;
				test_io_2_level = !test_io_2_level;
				R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_13_PIN_13, test_io_2_level);
			#endif
		}
	}
}

