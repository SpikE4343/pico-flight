// motor_output_dshot
//
#include <stdio.h>
#include <stdlib.h>

#include "motor_output.h"


BEGIN_DEF_DV_ARRAY(tdv_motor_output)
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.0.out.v", "Normalized motor throttle value sent to mixer", f32, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.1.out.v", "Normalized motor throttle value sent to mixer", f32, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.2.out.v", "Normalized motor throttle value sent to mixer", f32, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.3.out.v", "Normalized motor throttle value sent to mixer", f32, Tdm_RW | Tdm_realtime)
END_DEF_DV_ARRAY();


BEGIN_DEF_DV_ARRAY(tdv_motor_out_cmd)
  DEF_DV_ARRAY_ITEM_NAMED(0, "motor.0.out.cmd", "Native motor throttle value sent to esc", u16, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0, "motor.1.out.cmd", "Native motor throttle value sent to esc", u16, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0, "motor.2.out.cmd", "Native motor throttle value sent to esc", u16, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0, "motor.3.out.cmd", "Native motor throttle value sent to esc", u16, Tdm_RW | Tdm_realtime)
END_DEF_DV_ARRAY();

BEGIN_DEF_DV_ARRAY(tdv_motor_output_pin)
  DEF_DV_ARRAY_ITEM_NAMED(0, "motor.0.pin", "Gpio pin to use for communication", u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(1, "motor.1.pin", "Gpio pin to use for communication", u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(2, "motor.2.pin", "Gpio pin to use for communication", u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(3, "motor.3.pin", "Gpio pin to use for communication", u8, Tdm_RW | Tdm_config)
END_DEF_DV_ARRAY();


DEF_DATA_VAR(tdv_motor_output_min, 1.0f,
             "motor.output.min",
             "Minimum motor output value scalar",
             f32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_motor_output_idle, 0.05f,
             "motor.output.idle",
             "Motor idle scalar of total motor range",
             f32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_motor_output_max, 0.1f,
             "motor.output.max",
             "Maximum motor output value scalar",
             f32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_motor_output_enabled, 0,
             "motor.output.enabled",
             "Enable or disable all motor output",
             b8, Tdm_RW | Tdm_config);



