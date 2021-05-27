#ifndef data_vars_INCLUDED
#define data_vars_INCLUDED

#include "telemetry.h"
#include "gyro_vars.h"
#include "fc_vars.h"
#include "rc_vars.h"


void data_vars_init();

// Meant as a place holder for systems with a small number of data vars. 

// *****  Please break out groups into a new header file to maintain clairity   *****

DECL_EXTERN_DATA_VAR(tdv_telemetry_sample_buffer_count);
DECL_EXTERN_DATA_VAR(tdv_telemetry_val_count);
DECL_EXTERN_DATA_VAR(tdv_next_desc_send);
DECL_EXTERN_DATA_VAR(tdv_telemetry_queue);
DECL_EXTERN_DATA_VAR(tdv_telemetry_update_us);

// motor_mixer
DECL_EXTERN_DV_ARRAY(tdv_motor0_mix);
DECL_EXTERN_DV_ARRAY(tdv_motor1_mix);
DECL_EXTERN_DV_ARRAY(tdv_motor2_mix);
DECL_EXTERN_DV_ARRAY(tdv_motor3_mix);


DECL_EXTERN_DV_ARRAY( tdv_mixer_input );

DECL_EXTERN_DV_ARRAY(tdv_motor_direction);


// motor_common
DECL_EXTERN_DV_ARRAY(tdv_motor_direction);
DECL_EXTERN_DATA_VAR(tdv_motor_count);  
DECL_EXTERN_DATA_VAR( tdv_motor_startup_delay_ms );


// motor_output
DECL_EXTERN_DV_ARRAY( tdv_motor_output) ;
DECL_EXTERN_DV_ARRAY( tdv_motor_output_pin );

DECL_EXTERN_DATA_VAR( tdv_motor_output_min );
DECL_EXTERN_DATA_VAR( tdv_motor_output_idle );
DECL_EXTERN_DATA_VAR( tdv_motor_output_max );
DECL_EXTERN_DATA_VAR( tdv_motor_output_rate );


//osd
DECL_EXTERN_DATA_VAR(tdv_osd_pin_sync);
DECL_EXTERN_DATA_VAR(tdv_osd_pin_out);


#endif