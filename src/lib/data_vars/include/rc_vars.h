#ifndef rc_vars_INCLUDED
#define rc_vars_INCLUDED

#include "telemetry.h"

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

#endif