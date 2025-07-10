#include <stdio.h>
#include "m_tick.h"
#include "m_parameter.h"

m_tick_unit_t m_tick_unit;

void tick_execute()
{
	if (m_tick_unit.spd_tick_cnt != 0) m_tick_unit.spd_tick_cnt -- ;
	if (m_tick_unit.boost_charging_tick_cnt != 0) m_tick_unit.boost_charging_tick_cnt -- ;
	if (m_tick_unit.rotor_prepos1_tick_cnt != 0) m_tick_unit.rotor_prepos1_tick_cnt -- ;
	if (m_tick_unit.rotor_prepos2_tick_cnt != 0) m_tick_unit.rotor_prepos2_tick_cnt -- ;
	if (m_tick_unit.drag_tick_cnt != 0) m_tick_unit.drag_tick_cnt -- ;
	if (m_tick_unit.iq_calib_tick_cnt != 0) m_tick_unit.iq_calib_tick_cnt -- ;
	if (m_tick_unit.spd_loop_tick_cnt != 0) m_tick_unit.spd_loop_tick_cnt -- ;
	if (m_tick_unit.current_static_error_tick_cnt != 0) m_tick_unit.current_static_error_tick_cnt -- ;
	if (m_tick_unit.test_current_tick_cnt != 0) m_tick_unit.test_current_tick_cnt -- ;
	if (m_tick_unit.speed_set_value_tick_cnt != 0) m_tick_unit.speed_set_value_tick_cnt -- ;
}


