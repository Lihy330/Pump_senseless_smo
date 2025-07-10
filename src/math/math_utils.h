#ifndef __MATH_UTILS_H__
#define __MATH_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

int16_t m_obs_digital_LPF(int16_t q15_klpf, int16_t q15_z_n, int16_t q15_e_n_1);
int16_t	m_arctan_calculate(int16_t q15_e_alpha, int16_t q15_e_beta);


#ifdef _cplusplus
}
#endif

#endif
