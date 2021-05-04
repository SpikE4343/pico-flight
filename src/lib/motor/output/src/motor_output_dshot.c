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

DEF_DATA_VAR(tdv_motor_output_rate, MOTOR_DSHOT600, 
  "motor.output.rate",
  "Dshot output data rate",
  u32, Tdm_RW | Tdm_config);

