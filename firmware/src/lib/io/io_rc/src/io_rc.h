#ifndef __io_rc_h_INCLUDED__
#define __io_rc_h_INCLUDED__

#include "vector.h"
#include "stdbool.h"
#include "math_fixed.h"
#include "telemetry.h"

#define INPUT_CONTROL_ROLL 0
#define INPUT_CONTROL_PITCH 1
#define INPUT_CONTROL_YAW 2
#define INPUT_CONTROL_TROTTLE 3
#define NUM_AXIS_CONTROLS 4
#define INPUT_CONTROL_ARM 4

#define MAX_INPUT_CONTROL 32


void io_rc_init();
void io_rc_update();

bool io_rc_rx_buffer_empty();
bool io_rc_rx_buffer_full();
uint8_t io_rc_rx_buffer_read();
void io_rc_rx_buffer_write(uint8_t data);
uint8_t* io_rc_rx_buffer_bytes_written(uint8_t size);

void io_rc_protocol_init();
void io_rc_protocol_update();

void io_rc_rx_dma_init(uint8_t uart);

#endif