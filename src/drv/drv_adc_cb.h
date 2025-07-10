#ifndef __DRV_ADC_CB_H__
#define __DRV_ADC_CB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

#define ADC_GROUP0_NUM				(8)
#define ADC_GROUP1_NUM				(3)

typedef struct
{
	uint16_t instant_value;
	uint16_t average_value;
	uint32_t sum_value;
	uint16_t count_value;
	uint16_t filter_value;
	
}adcx_ch_type;

typedef struct
{
	adcx_ch_type u_current;
	adcx_ch_type v_current;
	adcx_ch_type w_current;
	
	// 三相电流的静态误差ADC值（值的范围是 0-4096）
	uint16_t ia_static_error;
	uint16_t ib_static_error;
	uint16_t ic_static_error;
	
	adcx_ch_type u_voltage;
	adcx_ch_type v_voltage;
	adcx_ch_type w_voltage;
	adcx_ch_type bus_current;
	adcx_ch_type bus_voltage;
	adcx_ch_type u_bemf;
	adcx_ch_type v_bemf;
	adcx_ch_type w_bemf;
	adcx_ch_type over_current_voltage;
	adcx_ch_type speed_voltage;
	adcx_ch_type ntc_voltage;
	uint16_t adc_group0_original_data[ADC_GROUP0_NUM];
	uint16_t adc_group1_original_data[ADC_GROUP1_NUM];
		
}adc_unit_t;

extern adc_unit_t adc_unit;

#ifdef _cplusplus
}
#endif

#endif