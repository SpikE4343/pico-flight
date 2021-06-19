#ifndef __motor_output_dshot_h_INCLUDED__
#define __motor_output_dshot_h_INCLUDED__

#define MOTOR_DSHOT1200 1200
#define MOTOR_DSHOT600 600
#define MOTOR_DSHOT300 300
#define MOTOR_DSHOT150 150

#define MOTOR_BIT_0 7
#define MOTOR_BIT_1 15
#define MOTOR_BITLENGTH 20

#define MOTOR_MIN_OUTPUT 48
#define MOTOR_MAX_OUTPUT 2047

#define MOTOR_OUTPUT_RANGE (MOTOR_MAX_OUTPUT - MOTOR_MIN_OUTPUT)

typedef struct
{
  union
  {
    struct
    {
      uint8_t crc : 4;
      uint8_t telemetryReq : 1;
      uint16_t throttle : 11;
    };

    uint16_t value;
  };
} __attribute__((__packed__)) DshotPacket_t;

#define DSHOT_PACKET_SIZE_BITS (sizeof(DshotPacket_t) * 8)

uint8_t dshotPacketCrc(uint16_t packet);
DshotPacket_t dshotBuildPacket(uint16_t throttle);

#endif