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

DECL_EXTERN_DV_ARRAY(tdv_rc_input);
DECL_EXTERN_DV_ARRAY(tdv_rc_mapping);

DECL_EXTERN_DATA_VAR(tdv_rc_signal_lost);
DECL_EXTERN_DATA_VAR(tdv_rc_failsafe);
DECL_EXTERN_DATA_VAR(tdv_rc_frames_recv);
DECL_EXTERN_DATA_VAR(tdv_rc_rssi);
DECL_EXTERN_DATA_VAR(tdv_rc_packet_loss);
DECL_EXTERN_DATA_VAR(tdv_rc_recv_state);

DECL_EXTERN_DATA_VAR(tdv_rc_uart_pins_tx);
DECL_EXTERN_DATA_VAR(tdv_rc_uart_pins_rx);
DECL_EXTERN_DATA_VAR(tdv_rc_uart_baud);
DECL_EXTERN_DATA_VAR(tdv_rc_uart_id);

void inputRxInit();
void inputRxUpdate();

void ioRCInitVars();


#endif