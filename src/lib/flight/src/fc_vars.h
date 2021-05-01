#ifndef fc_vars_INCLUDED
#define fc_vars_INCLUDED

#include "telemetry.h"


DECL_EXTERN_DATA_VAR(tdv_fc_state)
DECL_EXTERN_DATA_VAR(tdv_fc_gyro_updates)

DECL_EXTERN_DATA_VAR(tdv_fc_control_updates)
DECL_EXTERN_DATA_VAR(tdv_fc_core0_counter)
DECL_EXTERN_DATA_VAR(tdv_fc_core1_counter)
DECL_EXTERN_DATA_VAR(tdv_fc_gyro_update_us)


DECL_EXTERN_DATA_VAR(tdv_fc_armed);  
DECL_EXTERN_DATA_VAR(tdv_motor_count);    

DECL_EXTERN_DV_ARRAY(tdv_fc_rates_raw);
DECL_EXTERN_DV_ARRAY(tdv_fc_rates_filtered);
DECL_EXTERN_DV_ARRAY(tdv_fc_motor_output);

#endif