
#include <string.h>
#include <math.h>
#include <assert.h>
#include <malloc.h>
#include <stdio.h>

#include "hardware/timer.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/uart.h"


#include "telemetry.h"
#include "telemetry_native.h"

typedef struct
{
  uint8_t dmaId;
  uint8_t dmaMask;
  volatile uint32_t sendingCount;
  telemetry_native_send_callback_t sendCallback;

} TelemetryNativeState_t;

static TelemetryNativeState_t s;


bool telemetry_native_sending()
{
  return dma_channel_is_busy(s.dmaId);
}

static void __time_critical_func(dma_complete_handler)()
{
  if (dma_hw->ints1 & s.dmaMask)
  {
    // clear IRQ
    dma_hw->ints1 = s.dmaMask;
    if(s.sendCallback)
      s.sendCallback(s.sendingCount);

    s.sendingCount = 0;
  }
}

static void dma_init(uint8_t* packets)
{
  s.dmaId = dma_claim_unused_channel(true);
  s.dmaMask = 1u << s.dmaId;

  //irq_add_shared_handler(DMA_IRQ_0, dma_complete_handler, PICO_DEFAULT_IRQ_PRIORITY+8);

  dma_channel_config c = dma_channel_get_default_config(s.dmaId);
  channel_config_set_dreq(&c, DREQ_UART0_TX);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_read_increment(&c, true);
  dma_channel_configure(
      s.dmaId,
      &c,
      &uart_get_hw(uart0)->dr,
      packets,
      0,
      false);

  //dma_channel_set_irq0_enabled(s.dmaId, true);
  // s.sendCallback = 0;
}

static void dma_send(uint32_t sendLength, uint8_t* packets, int packetCount)
{
  if(dma_channel_is_busy(s.dmaId))
    return;

  s.sendingCount = packetCount;

  dma_channel_set_read_addr(s.dmaId, packets, false);
  dma_channel_set_trans_count(s.dmaId, sendLength, true);
}

void telemetry_native_init(telemetry_native_send_callback_t sendCB)
{
  dma_init(NULL);
  s.sendCallback = sendCB;
}


void telemetry_native_send(int length, uint8_t* packets, int packetCount)
{
  dma_send(length, packets, packetCount);
}

void telemetry_native_recv(int max)
{
  while(uart_is_readable(uart0) && max-- > 0)
  {
    uint8_t rd = uart_getc(uart0);
    telemetry_recv(rd);
  }
}

