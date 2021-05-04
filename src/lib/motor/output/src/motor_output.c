// motor_output_dshot
//
#include <stdio.h>
#include <stdlib.h>

#include "motor_output.h"


BEGIN_DEF_DV_ARRAY(tdv_motor_output)
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.0.output", "Normalized motor throttle value sent to mixer", f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.1.output", "Normalized motor throttle value sent to mixer", f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.2.output", "Normalized motor throttle value sent to mixer", f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.3.output", "Normalized motor throttle value sent to mixer", f32, Tdm_RW)
END_DEF_DV_ARRAY();

BEGIN_DEF_DV_ARRAY(tdv_motor_output_pin)
  DEF_DV_ARRAY_ITEM_NAMED(6, "motor.0.pin", "Gpio pin to use for communication", u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(7, "motor.1.pin", "Gpio pin to use for communication", u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(8, "motor.2.pin", "Gpio pin to use for communication", u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(9, "motor.3.pin", "Gpio pin to use for communication", u8, Tdm_RW | Tdm_config)
END_DEF_DV_ARRAY();


DEF_DATA_VAR(tdv_motor_output_min, 1.0f,
             "motor.output.min",
             "Minimum motor output value scalar",
             f32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_motor_output_idle, 0.05f,
             "motor.output.idle",
             "Motor idle scalar of total motor range",
             f32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_motor_output_max, 1.0f,
             "motor.output.max",
             "Maximum motor output value scalar",
             f32, Tdm_RW | Tdm_config);



