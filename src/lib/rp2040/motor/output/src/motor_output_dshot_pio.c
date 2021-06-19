// motor_output_dshot
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/sem.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#include "motor_output_dshot_pio.h"
#include "dshot_parallel.pio.h"
#include "math_util.h"
#include "data_vars.h"

typedef struct
{
  uint8_t dmaId;
} MotorOutputState_t;

static MotorOutputState_t s;
// 16 bits
//uint8_t bitPlane[sizeof(DshotPacket_t)];

// 8F
// 1000 1111
//

uint8_t dshotPacketBitPlanes[DSHOT_PACKET_SIZE_BITS];

// ---------------------------------------------------------------
void dshot_update_bitplanes(uint8_t *bitPlane, DshotPacket_t *packets)
{
  int bitCount = DSHOT_PACKET_SIZE_BITS;
  memset(bitPlane, 0, MAX_MOTORS*DSHOT_PACKET_SIZE_BITS);

  // for each packet
  for (int p = 0; p < MAX_MOTORS; ++p)
  {
    uint16_t value = packets[p].value;

    // write each bit of the packet to the correct bit plane
    for (int b = bitCount - 1; b >= 0; --b)
    {
      // copy a single bit from each packet into a single plane byte
      uint16_t bit_mask = 1 << (b);

      uint8_t isset = (value & bit_mask) ? 1 : 0;

      bitPlane[bitCount - 1 - b] |= isset << (p);
    }
  }
}

// ---------------------------------------------------------------
static void dma_init(PIO pio, uint sm)
{
  s.dmaId = dma_claim_unused_channel(true);

  // main DMA channel outputs 8 word fragments, and then chains back to the chain channel
  dma_channel_config c = dma_channel_get_default_config(s.dmaId);
  channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_read_increment(&c, true);
  dma_channel_configure(
      s.dmaId,
      &c,
      &pio->txf[sm],
      dshotPacketBitPlanes,
      DSHOT_PACKET_SIZE_BITS, // 8 words for 8 bit planes
      false);
}

// ---------------------------------------------------------------
void motorOutputInit()
{
  
  printf("motor init begin\n");


  memset(dshotPacketBitPlanes, 0, sizeof(dshotPacketBitPlanes));

  PIO pio = pio0;
  int sm = 0;
  uint offset = pio_add_program(pio, &dshot_parallel_program);

  dshot_parallel_program_init(pio, sm, offset, tdv_motor_output_pin[0].v.u8, MAX_MOTORS, tdv_motor_output_rate.v.u32 * 1000);

  dma_init(pio, sm);
  printf("motor init end\n");
}

// ---------------------------------------------------------------
void motorOutputSet(bool enabled, TDataVar_t *output)
{
  DshotPacket_t packets[MOTOR_MAX_OUTPUT];
  memset(packets, 0, sizeof(packets));

  if( enabled )
  {
    uint16_t idle = (uint16_t)(tdv_motor_output_idle.v.f32*MOTOR_MAX_OUTPUT);

    for (int m = 0; m < MAX_MOTORS; ++m)
    {
      uint16_t motorValue = math_clamp(
        MOTOR_MIN_OUTPUT, 
        (uint16_t)( idle + output[m].v.f32 * MOTOR_MAX_OUTPUT),
        MOTOR_MAX_OUTPUT
      );

      packets[m] = dshotBuildPacket(motorValue);
      tdv_motor_out_cmd[m].v.u16 = packets[m].value;
    }
  }
  else
  {
    for (int m = 0; m < MAX_MOTORS; ++m)
    {
      tdv_motor_out_cmd[m].v.u16 = packets[m].value;
    }
  }

  dshot_update_bitplanes(dshotPacketBitPlanes, packets);

  dma_channel_hw_addr(s.dmaId)->al1_read_addr = (uintptr_t)dshotPacketBitPlanes;
  dma_channel_hw_addr(s.dmaId)->al1_transfer_count_trig = DSHOT_PACKET_SIZE_BITS;
}
