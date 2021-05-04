#include "io_rc.h"
#include "data_vars.h"

DEF_DATA_VAR(tdv_rc_signal_lost, false, 
  "rc.signalLost",
  "True when input controls loses connection",
  b8, Tdm_RW);

DEF_DATA_VAR(tdv_rc_failsafe, false, 
  "rc.failsafe",
  "True when rc control system indicates failsafe",
  b8, Tdm_RW);

DEF_DATA_VAR(tdv_rc_frames_recv, 0, 
  "rc.frames.recv",
  "Count of rc data frames received",
  u32, Tdm_read);

DEF_DATA_VAR(tdv_rc_packet_loss, 0, 
  "rc.packet_loss",
  "Packets loss indicator from rx device",
  u32, Tdm_read);

DEF_DATA_VAR(tdv_rc_rssi, 0, 
  "rc.rssi",
  "Receive signal strengh indicator from rx device",
  i8, Tdm_read);

DEF_DATA_VAR(tdv_rc_uart_id, 1, 
  "rc.uart.id",
  "Id of uart to use",
  u8, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_rc_uart_pins_tx, 21, 
  "rc.uart.pins.tx",
  "Id of gpio pin to use for transmit",
  u8, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_rc_uart_pins_rx, 20, 
  "rc.uart.pins.rx",
  "Id of gpio pin to use for receive",
  u8, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_rc_uart_baud, 115200,
  "rc.uart.baud",
  "Baud rate to use for rc communication",
  u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_rc_recv_state, 0,
  "rc.recv.state",
  "Baud rate to use for rc communication",
  u8, Tdm_RW);

#define rc_control_name "rc.input"
#define rc_control_desc "RC control normalized inputs"

BEGIN_DEF_DV_ARRAY( tdv_rc_input )
  DEF_DV_ARRAY_ITEM(0, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(1, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(2, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(3, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(4, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(5, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(6, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(7, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM(8, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(9, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(10, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(11, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(12, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(13, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(14, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(15, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM(16, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(17, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(18, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(19, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(20, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(21, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(22, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(23, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),

  DEF_DV_ARRAY_ITEM(24, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(25, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(26, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(27, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(28, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(29, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(30, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
  DEF_DV_ARRAY_ITEM(31, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW),
END_DEF_DV_ARRAY();


// map raw inputs --> control inputs
#define rc_mapping_name "rc.mapping"
#define rc_mapping_desc "RC input mapping to controls"

// AERT
BEGIN_DEF_DV_ARRAY( tdv_rc_mapping )
  DEF_DV_ARRAY_ITEM(0, 0, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(1, 1, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(2, 2, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(3, 3, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(4, 4, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(5, 5, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(6, 6, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(7, 7, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
END_DEF_DV_ARRAY();


