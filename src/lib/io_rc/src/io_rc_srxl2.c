/* SPI Slave example, sender (uses SPI master driver)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/structs/uart.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#include "math_util.h"
#include "io_rc.h"
#include "spm_srxl.h"

#define BUFFER_SIZE 256
#define BUFFER_WRAP_MASK 0x00FF

typedef struct
{
  InputRxConfig_t *config;
  uint32_t validFrames;
  ControlState_t controls;
  uint32_t readTime;
  uint32_t uniqueID;
  uart_inst_t *uart;

  struct
  {
    union
    {
      SrxlHeader header;
      uint8_t packet[SRXL_MAX_BUFFER_SIZE];
    };

    uint8_t packetLen;

    uint8_t buffer[BUFFER_SIZE];
    volatile uint16_t read;
    volatile uint16_t write;
    volatile uint16_t length;
    uint8_t state;
  } rx;
} InputRxState_t;

static InputRxState_t s;

void userProvidedFillSrxlTelemetry(SrxlTelemetryData *pTelemetry);
void userProvidedReceivedChannelData(SrxlChannelData *pChannelData, bool isFailsafeData);
void userProvidedSetBaud(uint8_t uart, uint32_t baudRate);
void userProvidedSendOnUart(uint8_t uart, uint8_t *pBuffer, uint8_t length);
void userProvidedHandleVtxData(SrxlVtxData *pVtxData);

#define RX_RESET 0
#define RX_FIND_MARKER 1
#define RX_LOOP_MARKER 2
#define RX_READ_HEADER 3
#define RX_READ_DATA 4
#define RX_PARSE 5

#define BAUD_RATE 400000 //115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define DMA_CHANNEL 1
// chain channel for configuring main dma channel to output from disjoint 8 word fragments of memory
#define DMA_CHANNEL_MASK (1u << DMA_CHANNEL)

static inline bool buffer_empty()
{
  return s.rx.length == 0;
}

static inline bool buffer_full()
{
  return s.rx.read == s.rx.write && s.rx.length > 0;
}

uint8_t buffer_read()
{
  if (buffer_empty())
    return 0;

  uint8_t data = s.rx.buffer[s.rx.read];
  s.rx.read = (s.rx.read + 1) & BUFFER_WRAP_MASK;
  --s.rx.length;
  //printf("rx data read: %02X, %u, %u, %u\n", data, s.rx.read, s.rx.write, s.rx.length);
  return data;
}

void buffer_write(uint8_t data)
{
  if (buffer_full())
  {
    //printf("full\n");
    return;
  }

  s.rx.buffer[s.rx.write] = data;
  s.rx.write = (s.rx.write + 1) & BUFFER_WRAP_MASK;
  ++s.rx.length;
  //printf("rx data write: %02X, %u\n", data, s.rx.length );
}

// TODO: investigate using dma for this transfer
void on_uart_rx()
{
  if (uart_is_readable(s.uart))
  {

    uint8_t data = ((uart_hw_t *const)s.uart)->dr;

    //uint8_t data = uart_getc(s.uart);

    buffer_write(data);
  }
}

void inputRxInit(InputRxConfig_t *_config)
{
  memset(&s, 0, sizeof(s));
  s.config = _config;
  s.uniqueID = rand();
  s.uart = s.config->uart ? uart1 : uart0;
  s.rx.read = s.rx.write = 0;
  memset(s.rx.buffer, 0, sizeof(s.rx.buffer));
  s.rx.state = RX_RESET;

  uart_init(s.uart, 2400);

  gpio_set_function(s.config->txPin, GPIO_FUNC_UART);
  gpio_set_function(s.config->rxPin, GPIO_FUNC_UART);

  int actual = uart_set_baudrate(s.uart, BAUD_RATE);
  assert(actual == BAUD_RATE);

  // Set UART flow control CTS/RTS, we don't want these, so turn them off
  uart_set_hw_flow(s.uart, false, false);

  // Set our data format
  uart_set_format(s.uart, DATA_BITS, STOP_BITS, PARITY);

  uart_set_fifo_enabled(s.uart, true);

  int UART_IRQ = !s.uart ? UART0_IRQ : UART1_IRQ;

  // And set up and enable the interrupt handlers
  irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
  irq_set_enabled(UART_IRQ, true);

  uart_set_irq_enables(s.uart, true, false);

  // dma_claim_mask(DMA_CHANNEL_MASK);

  //   // main DMA channel outputs 8 word fragments, and then chains back to the chain channel
  // dma_channel_config channel_config = dma_channel_get_default_config(DMA_CHANNEL);
  // channel_config_set_dreq(&channel_config, s.config->uart ? DREQ_UART1_RX : DREQ_UART0_RX);
  // //channel_config_set_irq_quiet(&channel_config, false);
  // channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_8);
  // channel_config_set_write_increment(&channel_config, true);
  // //channel_config_set_ring(&channel_config, false, 4);
  // dma_channel_configure(
  //   DMA_CHANNEL,
  //   &channel_config,
  //   s.rx.buffer,
  //   dshotPacketBitPlanes,
  //   DSHOT_PACKET_SIZE_BITS, // 8 words for 8 bit planes
  //   false
  //   );

  if (!srxlInitDevice(SRXL_DEVICE_ID, SRXL_DEVICE_PRIORITY, SRXL_DEVICE_INFO, s.uniqueID))
    return;

  // Init the SRXL bus: The bus index must always be < SRXL_NUM_OF_BUSES -- in this case, it can only be 0
  if (!srxlInitBus(0, 0, SRXL_BAUD_400000))
    return;
}

fixed_t inputGetControlValue(uint8_t control)
{
  return s.controls.values[control];
}

ControlState_t *inputGetControlState()
{
  return &s.controls;
}

int readData(int max)
{
  int read = 0;
  while (uart_is_readable(s.uart) && read < max)
  {
    uint8_t data = uart_getc(s.uart);
    ++read;
    buffer_write(data);
  }

  return read;
}

void print_packet()
{
  printf("[l:%u, r:%u, w:%u, p1:%u] ",
         s.rx.length,
         s.rx.read,
         s.rx.write,
         s.rx.packetLen);

  for (int i = 0; i < s.rx.packetLen; ++i)
    printf("%02X ", s.rx.packet[i]);

  printf("\n");
}

void inputRxUpdate()
{

  readData(BUFFER_SIZE - s.rx.length);

  while (!buffer_empty())
  {
    switch (s.rx.state)
    {
    case RX_RESET:
      s.rx.packetLen = 0;
      s.rx.state = RX_FIND_MARKER;
      break;

    case RX_FIND_MARKER:
    case RX_LOOP_MARKER:
    {
      if (buffer_empty())
        break;

      uint8_t data = buffer_read();

      // printf("rx state: %u, pl:%u, bl:%u, r:%u, w:%u, d:%02X\n\t",
      //        s.rx.state,
      //        s.rx.packetLen,
      //        s.rx.length,
      //        s.rx.read,
      //        s.rx.write,
      //        data);

      if (data == SPEKTRUM_SRXL_ID)
      {
        s.rx.packet[s.rx.packetLen++] = data;
        s.rx.state = RX_LOOP_MARKER;
        break;
      }
      else if (s.rx.state == RX_LOOP_MARKER)
      {
        s.rx.packet[s.rx.packetLen++] = data;
        s.rx.state = RX_READ_HEADER;
      }
      else
      {
        srxlRun(0, 4);
      }

      // print_packet();
    }
    break;

    case RX_READ_HEADER:
    {
      if (buffer_empty())
        break;

      uint8_t data = buffer_read();

      // printf("rx state: %u, pl:%u, bl:%u, r:%u, w:%u, d:%02X\n\t",
      //        s.rx.state,
      //        s.rx.packetLen,
      //        s.rx.length,
      //        s.rx.read,
      //        s.rx.write,
      //        data);

      s.rx.packet[s.rx.packetLen++] = data;
      s.rx.state = RX_READ_DATA;

      // print_packet();
    }
    break;

    case RX_READ_DATA:
    {

      while (!buffer_empty() && s.rx.header.length != s.rx.packetLen)
      {
        uint8_t data = buffer_read();
        s.rx.packet[s.rx.packetLen++] = data;
      }

      // printf("\t");
      // print_packet();

      if (s.rx.header.length != s.rx.packetLen)
        break;

      //print_packet();
      bool valid = srxlParsePacket(0, s.rx.packet, s.rx.header.length);
      //printf("parse packet: %u\n", valid);
      if(!valid)
        print_packet();

      s.rx.state = RX_RESET;
    }
    break;
    }
  }
}

void userProvidedSetBaud(uint8_t uart, uint32_t baudRate)
{
  if (uart != 0)
    return;

  printf("rx: srxl2: set buadrate: %u\n", baudRate);
  // int actual = uart_set_baudrate(s.uart, baudRate);
  // assert(actual == baudRate);
}

void userProvidedSendOnUart(uint8_t uart, uint8_t *pBuffer, uint8_t length)
{
  if (uart != 0)
    return;

  uart_write_blocking(s.uart, pBuffer, length);
}

// User-defined routine to populate telemetry data
void userProvidedFillSrxlTelemetry(SrxlTelemetryData *pTelemetry)
{
  // Copy in whatever telemetry data you wish to send back this cycle
  // You can fill it via pTelemetry...
  memset(pTelemetry->raw, 0, 16);
  // ... or directly access the global value srxlTelemData
  srxlTelemData.sensorID = 0;
}

// User-defined routine to use the provided channel data from the SRXL bus master
void userProvidedReceivedChannelData(SrxlChannelData *pChannelData, bool isFailsafeData)
{
  // Use the received channel data in whatever way desired.
  // The failsafe flag is set if the data is failsafe data instead of normal channel data.
  // You can directly access the last received packet data through pChannelData,
  // in which case the values are still packed into the beginning of the array of channel data,
  // and must be identified by the mask bits that are set (NOT recommended!).
  // This is mostly provided in case you want to know which channels were actually sent
  // during this packet:
  // uint8_t currentIndex = 0;
  // if(pChannelData->mask & 1)
  //     servoA_12bit = pChannelData->values[currentIndex++] >> 4;
  // if(pChannelData->mask & 2)
  //     servoB_12bit = pChannelData->values[currentIndex++] >> 4;

  // // The recommended method is to access all channel data through the global srxlChData var:
  // servoA_12bit = srxlChData.values[0] >> 4;
  // servoB_12bit = srxlChData.values[1] >> 4;

  //srxlChData.
  // // RSSI and frame loss data are also available:
  // if(srxlChData.rssi < -85 || (srxlChData.rssi > 0 && srxlChData.rssi < 10))
  //     EnterLongRangeModeForExample();

  // for (int i = 0; i < sizeof(srxlChData.values) >> 1; ++i)
  // {
  //   printf("channel[%u]=%d\n", srxlChData.values[i]);
  // }

  printf("channel: rssi: %d, fl: %u, fs:%u\n", srxlChData.rssi, srxlChData.frameLosses, isFailsafeData);

  s.controls.values[0].value = (((int32_t)srxlChData.values[0]) - 32767) << 14;
  s.controls.values[1].value = (((int32_t)srxlChData.values[1]) - 32767) << 14;
  s.controls.values[2].value = (((int32_t)srxlChData.values[2]) - 32767) << 14;
  s.controls.values[3].value = (((int32_t)srxlChData.values[3]) - 32767) << 14;
  s.controls.values[4].value = (((int32_t)srxlChData.values[4]) - 32767) << 14;

  printf("i: [%f, %f, %f, %f, %f\n",
    fixed_to_float(s.controls.values[0]),
    fixed_to_float(s.controls.values[1]),
    fixed_to_float(s.controls.values[2]),
    fixed_to_float(s.controls.values[3]),
    fixed_to_float(s.controls.values[4]));
}

void userProvidedHandleVtxData(SrxlVtxData *pVtxData)
{
  printf("vtx data\n");
}