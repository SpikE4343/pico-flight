#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"

#if PICO_ON_DEVICE
#include "hardware/structs/uart.h"
#endif
#include "io_rc.h"
#include "data_vars.h"


#define BUFFER_SIZE 256
#define BUFFER_WRAP_MASK 0x00FF

typedef struct
{
  uint32_t validFrames;
  uint32_t readTime;
  uint32_t uniqueID;
  uart_inst_t *uart;

  struct
  {
    uint8_t packetLen;

    uint8_t buffer[BUFFER_SIZE];
    volatile uint16_t read;
    volatile uint16_t write;
    volatile uint16_t length;
  } rx;
} io_rc_state_t;

static io_rc_state_t s;

#define BAUD_RATE 400000 //115200
//#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

// ---------------------------------------------------------------
inline bool io_rc_rx_buffer_empty()
{
  return s.rx.length == 0;
}

// ---------------------------------------------------------------
inline bool io_rc_rx_buffer_full()
{
  return s.rx.read == s.rx.write && s.rx.length > 0;
}

// ---------------------------------------------------------------
uint8_t io_rc_rx_buffer_read()
{
  if (io_rc_rx_buffer_empty())
    return 0;

  uint8_t data = s.rx.buffer[s.rx.read];
  s.rx.read = (s.rx.read + 1) & BUFFER_WRAP_MASK;
  --s.rx.length;
  //printf("rx data read: %02X, %u, %u, %u\n", data, s.rx.read, s.rx.write, s.rx.length);
  return data;
}

// ---------------------------------------------------------------
void io_rc_rx_buffer_write(uint8_t data)
{
  if (io_rc_rx_buffer_full())
  {
    return;
  }

  s.rx.buffer[s.rx.write] = data;
  s.rx.write = (s.rx.write + 1) & BUFFER_WRAP_MASK;
  ++s.rx.length;
  ++tdv_rc_uart_rx_bytes.v.u32;
}

// ---------------------------------------------------------------
uint8_t* io_rc_rx_buffer_bytes_written(uint8_t size)
{
  s.rx.write = (s.rx.write + size) & BUFFER_WRAP_MASK;
  s.rx.length += size;

  tdv_rc_uart_rx_bytes.v.u32 += size;

  return s.rx.buffer+s.rx.write;
}

// ---------------------------------------------------------------
void io_rc_init()
{
  memset(&s, 0, sizeof(s));
  s.uniqueID = rand();
  s.uart = tdv_rc_uart_id.v.u8 ? uart1 : uart0;
  s.rx.read = s.rx.write = 0;
  memset(s.rx.buffer, 0, sizeof(s.rx.buffer));

  io_rc_rx_dma_init(tdv_rc_uart_id.v.u8);

  // init uart
  uart_init(s.uart, BAUD_RATE);

  gpio_set_function(tdv_rc_uart_pins_rx.v.u8, GPIO_FUNC_UART);
  gpio_set_function(tdv_rc_uart_pins_tx.v.u8, GPIO_FUNC_UART);

  // int actual = uart_set_baudrate(s.uart, BAUD_RATE);
  // printf("baud %d\n", actual);
  // assert(actual == BAUD_RATE);

  // Set UART flow control CTS/RTS, we don't want these, so turn them off
  // uart_set_hw_flow(s.uart, false, false);

  // Set our data format
  // uart_set_format(s.uart, DATA_BITS, STOP_BITS, PARITY);

  // uart_set_fifo_enabled(s.uart, true);

  // int UART_IRQ = !s.uart ? UART0_IRQ : UART1_IRQ;

  // And set up and enable the interrupt handlers
  // irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
  // irq_set_enabled(UART_IRQ, true);

  //uart_set_irq_enables(s.uart, false, false);





  
  io_rc_protocol_init();
}

// ---------------------------------------------------------------
void io_rc_update()
{
  io_rc_protocol_update();
}

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_signal_lost, false, 
  "rc.signalLost",
  "True when input controls loses connection",
  b8, Tdm_RW);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_failsafe, false, 
  "rc.failsafe",
  "True when rc control system indicates failsafe",
  b8, Tdm_RW);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_frames_recv, 0, 
  "rc.frames.recv",
  "Count of rc data frames received",
  u32, Tdm_read | Tdm_realtime);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_packet_loss, 0, 
  "rc.packet_loss",
  "Packets loss indicator from rx device",
  u32, Tdm_read | Tdm_realtime);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_rssi, 0, 
  "rc.rssi",
  "Receive signal strengh indicator from rx device",
  i8, Tdm_read | Tdm_realtime);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_uart_id, 1, 
  "rc.uart.id",
  "Id of uart to use",
  u8, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_uart_pins_tx, 20, 
  "rc.uart.pins.tx",
  "Id of gpio pin to use for transmit",
  u8, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_uart_pins_rx, 21, 
  "rc.uart.pins.rx",
  "Id of gpio pin to use for receive",
  u8, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_uart_baud, 400000,
  "rc.uart.baud",
  "Baud rate to use for rc communication",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_recv_state, 0,
  "rc.recv.state",
  "Baud rate to use for rc communication",
  u8, Tdm_RW);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_uart_rx_bytes, 0,
  "rc.uart.rx",
  "Bytes read from rc uart",
  u32, Tdm_RW | Tdm_realtime);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_uart_tx_bytes, 0,
  "rc.uart.tx",
  "Bytes sent to rc uart",
  u32, Tdm_RW | Tdm_realtime);


// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_last_recv_us, 0,
  "rc.recv.last.us",
  "Last time data recv from rx",
  u32, Tdm_read);


// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_rc_recv_timeout, 1000,
  "rc.recv.timeout.ms",
  "Time without recieving rx packets before failsafe",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
#define rc_control_name "rc.input"
#define rc_control_desc "RC control normalized inputs"

BEGIN_DEF_DV_ARRAY( tdv_rc_input )
  DEF_DV_ARRAY_ITEM(0, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM(1, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM(2, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM(3, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM(4, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM(5, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM(6, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  DEF_DV_ARRAY_ITEM(7, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),

  // DEF_DV_ARRAY_ITEM(8, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(9, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(10, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(11, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(12, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(13, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(14, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(15, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),

  // DEF_DV_ARRAY_ITEM(16, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(17, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(18, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(19, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(20, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(21, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(22, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(23, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),

  // DEF_DV_ARRAY_ITEM(24, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(25, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(26, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(27, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(28, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(29, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(30, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
  // DEF_DV_ARRAY_ITEM(31, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
END_DEF_DV_ARRAY();

// ---------------------------------------------------------------
// map raw inputs --> control inputs
#define rc_mapping_name "rc.mapping"
#define rc_mapping_desc "RC input mapping to controls"

// AETR
BEGIN_DEF_DV_ARRAY( tdv_rc_mapping )
  DEF_DV_ARRAY_ITEM(0, 0, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(1, 1, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(2, 3, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(3, 2, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(4, 4, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(5, 5, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(6, 6, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
  DEF_DV_ARRAY_ITEM(7, 7, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
END_DEF_DV_ARRAY();


