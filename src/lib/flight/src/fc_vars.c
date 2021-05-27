

#include <stdlib.h>
#include "flight_controller.h"

#include "telemetry.h"

DEF_DATA_VAR(tdv_fc_state, 0, 
  "fc.state",
  "Flight Controller State",
  i32, Tdm_read);

DEF_DATA_VAR(tdv_fc_gyro_updates, 0, 
  "fc.gyro.updates",
  "Flight Controller number of times fc has updated internal state from gyro",
  u32, Tdm_read);


DEF_DATA_VAR(tdv_fc_gyro_roll_filtered, 0.0f,
  "fc.gyro.filtered.r",
  "Flight Controller copy of filtered gyro roll data",
  f32, Tdm_RW);

DEF_DATA_VAR(tdv_fc_gyro_pitch_filtered, 0.0f, 
  "fc.gyro.filtered.p", 
  "Flight Controller copy of filtered gyro pitch data", 
  f32, Tdm_RW);

DEF_DATA_VAR(tdv_fc_gyro_yaw_filtered, 0.0f, 
  "fc.gyro.filtered.y",
  "Flight Controller copy of filtered gyro yaw data",
  f32, Tdm_RW);

DEF_DATA_VAR(tdv_fc_control_updates, 0, 
  "fc.control.updates",
  "Flight Controller number of times fc has updated internal state from controls",
  u32, Tdm_read);

DEF_DATA_VAR(tdv_fc_core0_counter, 0, 
  "fc.core0.counter",
  "Flight Controller number of times core0 idle ",
  u32, Tdm_read);

DEF_DATA_VAR(tdv_fc_core1_counter, 0, 
  "fc.core1.counter",
  "Flight Controller number of times core1 idle",
  u32, Tdm_read);

DEF_DATA_VAR(tdv_fc_gyro_update_us, 0, 
  "fc.gyro.update.us",
  "Flight Controller number of times fc has updated internal state from gyro",
  u32, Tdm_read);

DEF_DATA_VAR(tdv_fc_armed, false, 
  "fc.armed",
  "True when craft motors are active, false otherwise",
  b8, Tdm_RW);

DEF_DATA_VAR(tdv_fc_update_rate_hz, 4000, 
  "fc.update_rate",
  "True when craft motors are active, false otherwise",
  u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_fc_telemetry_rate_hz, 60, 
  "fc.telemetry.rate",
  "Rate, in hertz, to send telemetry samples",
  u32, Tdm_RW | Tdm_config);

BEGIN_DEF_DV_ARRAY( tdv_fc_rates_raw )
  DEF_DV_ARRAY_ITEM(roll, 0.0f,
    "fc.gyro.raw",
    "Flight Controller copy of raw gyro roll data",
    f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM(pitch, 0.0f, 
    "fc.gyro.raw", 
    "Flight Controller copy of raw gyro pitch data", 
    f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM(yaw, 0.0f, 
    "fc.gyro.raw",
    "Flight Controller copy of raw gyro yaw data",
    f32, Tdm_RW)
END_DEF_DV_ARRAY();


BEGIN_DEF_DV_ARRAY( tdv_fc_rates_filtered )
  DEF_DV_ARRAY_ITEM(roll, 0.0f,
    "fc.gyro.filtered",
    "Flight Controller copy of filtered gyro roll data",
    f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM(pitch, 0.0f, 
    "fc.gyro.filtered", 
    "Flight Controller copy of filtered gyro pitch data", 
    f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM(yaw, 0.0f, 
    "fc.gyro.filtered",
    "Flight Controller copy of filtered gyro yaw data",
    f32, Tdm_RW)
END_DEF_DV_ARRAY();


BEGIN_DEF_DV_ARRAY( tdv_fc_inputs )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.input.roll", "Roll input", f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.input.pitch", "Pitch input", f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.input.yaw", "Yaw input", f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.input.throttle", "Throttle input", f32, Tdm_RW)
END_DEF_DV_ARRAY();



