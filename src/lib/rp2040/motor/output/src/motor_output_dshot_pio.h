#ifndef __motor_output_dshot_pio_h_INCLUDED__
#define __motor_output_dshot_pio_h_INCLUDED__

#include "motor_output.h"
#include "motor_output_dshot.h"

void dshot_update_bitplanes(uint8_t *bitPlane, DshotPacket_t *packets);

#endif