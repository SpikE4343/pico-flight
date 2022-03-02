
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "hardware/dma.h"
#include "hardware/irq.h"

#if PICO_ON_DEVICE
#include "hardware/structs/uart.h"
#endif


#include "math_util.h"
#include "io_rc.h"
#include "data_vars.h"


static int dmaId=0;
static int dmaMask=0;

#define READ_BYTES 1


// ---------------------------------------------------------------
void __time_critical_func(dma_complete_handler)() 
{
  if (!(dma_hw->ints0 & dmaMask))
    return;

  dma_hw->ints0 = dmaMask;

  int8_t* write = io_rc_rx_buffer_bytes_written(READ_BYTES);

  dma_channel_hw_addr(dmaId)->al1_write_addr = (uintptr_t)write;
  dma_channel_hw_addr(dmaId)->al1_transfer_count_trig = READ_BYTES;
}

// ---------------------------------------------------------------
void io_rc_rx_dma_init(uint8_t uart)
{
  dmaId = dma_claim_unused_channel(true);
  dmaMask = 1u << dmaId;

  
  // had to set to a lower priority than another shared handler
  // if both are highest priority it will cause a crash.
  // irq_add_shared_handler(
  //     DMA_IRQ_0, dma_complete_handler, PICO_DEFAULT_IRQ_PRIORITY);

  dma_channel_config c = dma_channel_get_default_config(dmaId);
  channel_config_set_dreq(&c,  uart ? DREQ_UART1_RX : DREQ_UART0_RX);
  // channel_config_set_irq_quiet(&c, false);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  // channel_config_set_write_increment(&c, true);
  channel_config_set_read_increment(&c, false);
  // channel_config_set_ring(&c, true, 8);
  
  int8_t* write = io_rc_rx_buffer_bytes_written(0);

  dma_channel_configure(
      dmaId,
      &c,
      (void*)write,
      &uart_get_hw(uart ? uart1 : uart0)->dr,
      READ_BYTES,
      false);
  
  irq_set_exclusive_handler(DMA_IRQ_0, dma_complete_handler);
  dma_channel_set_irq0_enabled(dmaId, true);
  irq_set_enabled(DMA_IRQ_0, true);
  

  dma_channel_hw_addr(dmaId)->al1_write_addr = (uintptr_t)write;
  dma_channel_hw_addr(dmaId)->al1_transfer_count_trig = READ_BYTES;

  assert(dma_channel_is_busy(dmaId));
}
