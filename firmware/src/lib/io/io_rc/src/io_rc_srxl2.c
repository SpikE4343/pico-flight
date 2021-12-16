
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "pico/stdlib.h"

#include "math_util.h"
#include "io_rc.h"
#include "data_vars.h"
#include "spm_srxl.h"
#include "system.h"

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
    union
    {
      SrxlHeader header;
      uint8_t packet[SRXL_MAX_BUFFER_SIZE];
    };

    uint8_t packetLen;
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




// ---------------------------------------------------------------
void io_rc_protocol_init()
{
  tdv_rc_recv_state.v.u8 = RX_RESET;

  if (!srxlInitDevice(SRXL_DEVICE_ID, SRXL_DEVICE_PRIORITY, SRXL_DEVICE_INFO, s.uniqueID))
    return;

  // Init the SRXL bus: The bus index must always be < SRXL_NUM_OF_BUSES -- in this case, it can only be 0
  if (!srxlInitBus(0, 0, SRXL_BAUD_400000))
    return;
}

// // ---------------------------------------------------------------
// void print_packet()
// {
//   printf("[l:%u, r:%u, w:%u, p1:%u] ",
//          s.rx.length,
//          s.rx.read,
//          s.rx.write,
//          s.rx.packetLen);

//   for (int i = 0; i < s.rx.packetLen; ++i)
//     printf("%02X ", s.rx.packet[i]);

//   printf("\n");
// }

// ---------------------------------------------------------------
void io_rc_protocol_update()
{
  while (!io_rc_rx_buffer_empty())
  {
    switch (tdv_rc_recv_state.v.u8)
    {
    // ---------------------------------------------------------------
    case RX_RESET:
      s.rx.packetLen = 0;
      tdv_rc_recv_state.v.u8 = RX_FIND_MARKER;
      break;

    // ---------------------------------------------------------------
    case RX_FIND_MARKER:
    case RX_LOOP_MARKER:
    {
      if (io_rc_rx_buffer_empty())
        break;

      uint8_t data = io_rc_rx_buffer_read();

      if (data == SPEKTRUM_SRXL_ID)
      {
        s.rx.packet[s.rx.packetLen++] = data;
        tdv_rc_recv_state.v.u8 = RX_LOOP_MARKER;
        break;
      }
      else if (tdv_rc_recv_state.v.u8 == RX_LOOP_MARKER)
      {
        s.rx.packet[s.rx.packetLen++] = data;
        tdv_rc_recv_state.v.u8 = RX_READ_HEADER;
      }
      else
      {
        // srxlRun(0, 4);
      }
    }
    break;

    // ---------------------------------------------------------------
    case RX_READ_HEADER:
    {
      if (io_rc_rx_buffer_empty())
        break;

      uint8_t data = io_rc_rx_buffer_read();

      s.rx.packet[s.rx.packetLen++] = data;
      tdv_rc_recv_state.v.u8 = RX_READ_DATA;
    }
    break;

    // ---------------------------------------------------------------
    case RX_READ_DATA:
    {

      while (!io_rc_rx_buffer_empty() && s.rx.header.length != s.rx.packetLen)
      {
        uint8_t data = io_rc_rx_buffer_read();
        s.rx.packet[s.rx.packetLen++] = data;
      }

      if (s.rx.header.length != s.rx.packetLen)
        break;

      bool valid = srxlParsePacket(0, s.rx.packet, s.rx.header.length);

      tdv_rc_packet_loss.v.u32 += !valid;

      tdv_rc_frames_recv.v.u32 += valid;

      tdv_rc_recv_state.v.u8 = RX_RESET;
    }
    break;
    }
  }

  srxlRun(0, 4);
}

// ---------------------------------------------------------------
void userProvidedSetBaud(uint8_t uart, uint32_t baudRate)
{
  if (uart != 0)
    return;

  // printf("rx: srxl2: set buadrate: %u\n", baudRate);
  // int actual = uart_set_baudrate(s.uart, baudRate);
  // assert(actual == baudRate);
}

// ---------------------------------------------------------------
void userProvidedSendOnUart(uint8_t uart, uint8_t *pBuffer, uint8_t length)
{
  if (uart != 0)
    return;

  uart_write_blocking(s.uart, pBuffer, length);
}

// ---------------------------------------------------------------
// User-defined routine to populate telemetry data
void userProvidedFillSrxlTelemetry(SrxlTelemetryData *pTelemetry)
{
  // Copy in whatever telemetry data you wish to send back this cycle
  // You can fill it via pTelemetry...
  memset(pTelemetry->raw, 0, 16);
  // ... or directly access the global value srxlTelemData
  srxlTelemData.sensorID = 0;
}

// ---------------------------------------------------------------
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
  // divdie by 8 for only 8 channels
  for (int i = 0; i < sizeof(srxlChData.values) >> 3; ++i)
  {
     tdv_rc_input[i].v.f32 = (srxlChData.values[i] - 32767) / 32767.0f;
  }

  tdv_rc_failsafe.v.b8 = isFailsafeData;
  tdv_rc_rssi.v.i8 = srxlChData.rssi;
  tdv_rc_last_recv_us.v.u32 = (uint32_t)system_time_us();
}

// ---------------------------------------------------------------
void userProvidedHandleVtxData(SrxlVtxData *pVtxData)
{
  printf("vtx data\n");
}
