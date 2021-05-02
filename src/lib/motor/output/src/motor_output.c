// motor_output_dshot
//
#include <stdio.h>
#include <stdlib.h>

#include "motor_output.h"
#include "motor_output_dshot.h"

uint8_t dshotPacketCrc(DshotPacket_t packet)
{
  unsigned chk = packet.value;
  unsigned sum = 0;

  for (int i = 0; i < 3; ++i)
  {
    sum ^= chk;
    chk >>= 4;
  }

  return sum & 0xF;
}

DshotPacket_t dshotBuildPacket(uint16_t throttle)
{
  DshotPacket_t packet;
  packet.throttle = throttle;
  packet.telemetryReq = 0;
  packet.crc = dshotPacketCrc(packet);
  return packet;
}

void motorOutputInitVars()
{
  telemetry_register_array(tdv_motor_output, 4);
  telemetry_register_array(tdv_motor_output_pin, 4);

  telemetry_register(&tdv_motor_startup_delay_ms);
  telemetry_register(&tdv_motor_output_min);
  telemetry_register(&tdv_motor_output_idle);
  telemetry_register(&tdv_motor_output_max);
  telemetry_register(&tdv_motor_output_rate);
}

BEGIN_DEF_DV_ARRAY(tdv_motor_output)
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.0.output", "Normalized motor throttle value sent to mixer", Tdt_f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.1.output", "Normalized motor throttle value sent to mixer", Tdt_f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.2.output", "Normalized motor throttle value sent to mixer", Tdt_f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "motor.3.output", "Normalized motor throttle value sent to mixer", Tdt_f32, Tdm_RW)
END_DEF_DV_ARRAY();

BEGIN_DEF_DV_ARRAY(tdv_motor_output_pin)
  DEF_DV_ARRAY_ITEM_NAMED(6, "motor.0.pin", "Gpio pin to use for communication", Tdt_u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(7, "motor.1.pin", "Gpio pin to use for communication", Tdt_u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(8, "motor.2.pin", "Gpio pin to use for communication", Tdt_u8, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(9, "motor.3.pin", "Gpio pin to use for communication", Tdt_u8, Tdm_RW | Tdm_config),
END_DEF_DV_ARRAY();

DEF_DATA_VAR(tdv_motor_startup_delay_ms, 10,
             "motor.output.startup_delay",
             "Milliseconds to wait before sending any output commands to motor controllers",
             Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_motor_output_min, 1.0f,
             "motor.output.min",
             "Minimum motor output value scalar",
             Tdt_f32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_motor_output_idle, 0.05f,
             "motor.output.idle",
             "Motor idle scalar of total motor range",
             Tdt_f32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_motor_output_min, 1.0f,
             "motor.output.max",
             "Maximum motor output value scalar",
             Tdt_f32, Tdm_RW | Tdm_config);
