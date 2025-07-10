#include <stdio.h>
#include "drv_pwm.h"
#include "../motor/m_parameter.h"

void drv_pwm_init()
{
	three_phase_duty_cycle_t three_phase_duty_cycle;
	R_GPT_THREE_PHASE_Open(g_three_phase0.p_ctrl, g_three_phase0.p_cfg);
	R_GPT_THREE_PHASE_Start(g_three_phase0.p_ctrl);
	
	// Set the three-phase duty cycle on the up bridge to 0
	// Naturally, the lower bridge is set to 1
	three_phase_duty_cycle.duty[0] = GPT_RELOAD_VALUE;
	three_phase_duty_cycle.duty[1] = GPT_RELOAD_VALUE;
	three_phase_duty_cycle.duty[2] = GPT_RELOAD_VALUE;
	

	// The duty_buffer is set to 0
	three_phase_duty_cycle.duty_buffer[0] = 0;
	three_phase_duty_cycle.duty_buffer[1] = 0;
	three_phase_duty_cycle.duty_buffer[2] = 0;

	R_GPT_THREE_PHASE_DutyCycleSet(g_three_phase0.p_ctrl, &three_phase_duty_cycle);
}

