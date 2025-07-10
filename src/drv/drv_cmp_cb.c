#include <stdio.h>
#include "drv_cmp_cb.h"

uint32_t cmp0_trigger_cnt = 0;


void comparator0_callback(comparator_callback_args_t * p_args)
{
		// Count the number of interruptions entered
		if (p_args->channel == 0)
		{
				cmp0_trigger_cnt ++ ;
		}
}

