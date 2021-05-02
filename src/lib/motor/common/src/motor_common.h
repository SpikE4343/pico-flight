#ifndef __motor_common_INCLUDED__
#define __motor_common_INCLUDED__

#include "telemetry.h"

#define MAX_MOTORS 4

void motorCommonInit();


DECL_EXTERN_DV_ARRAY(tdv_motor_direction);
DECL_EXTERN_DATA_VAR(tdv_motor_count);  

#endif