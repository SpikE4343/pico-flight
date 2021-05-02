

#include <stdlib.h>
#include "flight_controller.h"

#include "motor_output.h"
#include "telemetry.h"

DEF_DATA_VAR(tdv_fc_state, 0, 
  "fc.state",
  "Flight Controller State",
  Tdt_i32, Tdm_read);

DEF_DATA_VAR(tdv_fc_gyro_updates, 0, 
  "fc.gyro.updates",
  "Flight Controller number of times fc has updated internal state from gyro",
  Tdt_u32, Tdm_read);


DEF_DATA_VAR(tdv_fc_gyro_roll_filtered, 0.0f,
  "fc.gyro.filtered.r",
  "Flight Controller copy of filtered gyro roll data",
  Tdt_f32, Tdm_RW);

DEF_DATA_VAR(tdv_fc_gyro_pitch_filtered, 0.0f, 
  "fc.gyro.filtered.p", 
  "Flight Controller copy of filtered gyro pitch data", 
  Tdt_f32, Tdm_RW);

DEF_DATA_VAR(tdv_fc_gyro_yaw_filtered, 0.0f, 
  "fc.gyro.filtered.y",
  "Flight Controller copy of filtered gyro yaw data",
  Tdt_f32, Tdm_RW);

DEF_DATA_VAR(tdv_fc_control_updates, 0, 
  "fc.control.updates",
  "Flight Controller number of times fc has updated internal state from controls",
  Tdt_u32, Tdm_read);

DEF_DATA_VAR(tdv_fc_core0_counter, 0, 
  "fc.core0.counter",
  "Flight Controller number of times core0 idle ",
  Tdt_u32, Tdm_read);

DEF_DATA_VAR(tdv_fc_core1_counter, 0, 
  "fc.core1.counter",
  "Flight Controller number of times core1 idle",
  Tdt_u32, Tdm_read);

DEF_DATA_VAR(tdv_fc_gyro_update_us, 0, 
  "fc.gyro.update.us",
  "Flight Controller number of times fc has updated internal state from gyro",
  Tdt_u32, Tdm_read);

DEF_DATA_VAR(tdv_fc_armed, false, 
  "fc.armed",
  "True when craft motors are active, false otherwise",
  Tdt_bool, Tdm_RW);

DEF_DATA_VAR(tdv_fc_update_rate_hz, 4000, 
  "fc.update_rate",
  "True when craft motors are active, false otherwise",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_fc_telemetry_rate_hz, 60, 
  "fc.telemetry.rate",
  "Rate, in hertz, to send telemetry samples",
  Tdt_u32, Tdm_RW | Tdm_config);

BEGIN_DEF_DV_ARRAY( tdv_fc_rates_raw )
  DEF_DV_ARRAY_ITEM("roll", 0.0f,
    "fc.gyro.raw",
    "Flight Controller copy of raw gyro roll data",
    Tdt_f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM("pitch", 0.0f, 
    "fc.gyro.raw", 
    "Flight Controller copy of raw gyro pitch data", 
    Tdt_f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM("yaw", 0.0f, 
    "fc.gyro.raw",
    "Flight Controller copy of raw gyro yaw data",
    Tdt_f32, Tdm_RW)
END_DEF_DV_ARRAY();


BEGIN_DEF_DV_ARRAY( tdv_fc_rates_filtered )
  DEF_DV_ARRAY_ITEM("roll", 0.0f,
    "fc.gyro.filtered",
    "Flight Controller copy of filtered gyro roll data",
    Tdt_f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM("pitch", 0.0f, 
    "fc.gyro.filtered", 
    "Flight Controller copy of filtered gyro pitch data", 
    Tdt_f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM("yaw", 0.0f, 
    "fc.gyro.filtered",
    "Flight Controller copy of filtered gyro yaw data",
    Tdt_f32, Tdm_RW)
END_DEF_DV_ARRAY();

void flightInitVars()
{
  telemetry_register(&tdv_fc_state);
  telemetry_register(&tdv_fc_gyro_updates);

  telemetry_register(&tdv_fc_control_updates);

  telemetry_register(&tdv_fc_core0_counter);
  telemetry_register(&tdv_fc_core1_counter);
  telemetry_register(&tdv_fc_gyro_update_us);  

  telemetry_register(&tdv_fc_armed);  
  telemetry_register(&tdv_motor_count);    

  telemetry_register(&tdv_fc_update_rate_hz);
  telemetry_register(&tdv_fc_telemetry_rate_hz);

  telemetry_register_array(tdv_fc_rates_raw, sizeof(tdv_fc_rates_raw)/sizeof(TDataVar_t));
  telemetry_register_array(tdv_fc_rates_filtered, sizeof(tdv_fc_rates_filtered)/sizeof(TDataVar_t));
}

