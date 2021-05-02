
#include <stdio.h>
#include <stdlib.h>

#include "motor_common.h"


void motorCommonInit()
{

  telemetry_register_array(tdv_motor_direction, 4);

  telemetry_register(&tdv_motor_count);

}

#define motor_direction_name "motor_direction"
#define motor_direction_desc "Motor direction value"
BEGIN_DEF_DV_ARRAY( tdv_motor_direction )
  DEF_DV_ARRAY_ITEM(0, 1, motor_direction_name, motor_direction_desc, Tdt_u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(1, 1, motor_direction_name, motor_direction_desc, Tdt_u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(2, 1, motor_direction_name, motor_direction_desc, Tdt_u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(3, 1, motor_direction_name, motor_direction_desc, Tdt_u8, Tdm_RW | Tdm_config),
END_DEF_DV_ARRAY();

DEF_DATA_VAR(tdv_motor_count, 4, 
  "motor.count",
  "Number of active motors",
  Tdt_u8, Tdm_RW | Tdm_config);
