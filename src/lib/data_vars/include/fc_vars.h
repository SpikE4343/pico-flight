#ifndef fc_vars_INCLUDED
#define fc_vars_INCLUDED

#include "telemetry.h"


DECL_EXTERN_DATA_VAR(tdv_fc_state);
DECL_EXTERN_DATA_VAR(tdv_fc_gyro_updates);

DECL_EXTERN_DATA_VAR(tdv_fc_control_updates);
DECL_EXTERN_DATA_VAR(tdv_fc_core0_counter);
DECL_EXTERN_DATA_VAR(tdv_fc_core1_counter);
DECL_EXTERN_DATA_VAR(tdv_fc_gyro_update_us);

DECL_EXTERN_DATA_VAR(tdv_fc_telemetry_rate_hz);
DECL_EXTERN_DATA_VAR(tdv_fc_update_rate_hz);

DECL_EXTERN_DATA_VAR(tdv_fc_armed);  

DECL_EXTERN_DV_ARRAY(tdv_fc_rates_raw);
DECL_EXTERN_DV_ARRAY(tdv_fc_rates_filtered);


DECL_EXTERN_DV_ARRAY(tdv_fc_inputs);
DECL_EXTERN_DV_ARRAY(tdv_fc_attitude_outputs);

DECL_EXTERN_DV_ARRAY(tdv_fc_pidf_k_roll);
DECL_EXTERN_DV_ARRAY(tdv_fc_pidf_k_pitch);
DECL_EXTERN_DV_ARRAY(tdv_fc_pidf_k_yaw);

DECL_EXTERN_DV_ARRAY(tdv_fc_pidf_v_roll);
DECL_EXTERN_DV_ARRAY(tdv_fc_pidf_v_pitch);
DECL_EXTERN_DV_ARRAY(tdv_fc_pidf_v_yaw);

DECL_EXTERN_DV_ARRAY(tdv_fc_pid_sum);
DECL_EXTERN_DV_ARRAY(tdv_fc_pid_sp);
DECL_EXTERN_DV_ARRAY(tdv_fc_pid_sp_error);

DECL_EXTERN_DV_ARRAY(tdv_fc_pid_sp_delta);
DECL_EXTERN_DV_ARRAY(tdv_fc_pid_pv_error);
DECL_EXTERN_DV_ARRAY(tdv_fc_pid_pv);

#endif