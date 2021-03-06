
#include <stdio.h>
#include <stdlib.h>

#include "motor_common.h"

void motorCommonInit()
{

}


#define motor_direction_name "motor_direction"
#define motor_direction_desc "Motor direction value"
BEGIN_DEF_DV_ARRAY( tdv_motor_direction )
  DEF_DV_ARRAY_ITEM_NAMED(1, "motor.0.dir", motor_direction_desc, u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(1, "motor.1.dir", motor_direction_desc, u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(1, "motor.2.dir", motor_direction_desc, u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(1, "motor.3.dir", motor_direction_desc, u8, Tdm_RW | Tdm_config),
END_DEF_DV_ARRAY();

DEF_DATA_VAR(tdv_motor_count, 4, 
  "motor.count",
  "Number of active motors",
  u8, Tdm_RW | Tdm_config);



DEF_DATA_VAR(tdv_motor_startup_delay_ms, 10,
             "motor.output.startup_delay",
             "Milliseconds to wait before sending any output commands to motor controllers",
             u32, Tdm_RW | Tdm_config);
