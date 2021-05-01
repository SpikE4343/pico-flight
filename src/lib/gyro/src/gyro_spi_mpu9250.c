/* SPI Slave example, sender (uses SPI master driver)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "math_util.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "gyro_spi_mpu9250.h"
#include "telemetry.h"





DEF_DATA_VAR(tdv_gyro_lpfCutoffHz, 50, 
  "gyro.filterHz",
  "Gyro filter cutoff in hertz",
  Tdt_i32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_resolution, 50, 
  "gyro.scale",
  "Gyro scale",
  Tdt_f32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_dataReadyInterruptPin, 50, 
  "gyro.filterHz",
  "Gyro filter cutoff in hertz",
  Tdt_i32, Tdm_RW | Tdm_config);


typedef spi_inst_t *spi_t;

typedef struct 
{
  uint8_t id;
  uint8_t mask;
} GyroDma_t;

typedef struct
{
  spi_t spi;
  GyroState_t gyro;
  uint8_t gyroBuffer[7];
  uint8_t gyroBufferRx[7];
  GyroDma_t dma_tx;
  GyroDma_t dma_rx;

  uint32_t startCount;
  uint32_t completeCount;

  gyro_update_complete_callback_t completeCallback;

  Vector3i32_t rates;  

} GyroLocalState_t;


static GyroLocalState_t s;

static uint8_t debug = 1;

static void gyroUpdateState();

static inline void cs_select(int enable) {
    // asm volatile("nop \n nop \n nop");
    // gpio_put(s.gyro.config->spi.selectPin, enable);  // Active low
    // asm volatile("nop \n nop \n nop");
}

static void __time_critical_func(dma_complete_handler)()
  
{
  if (!(dma_hw->ints1 & s.dma_rx.mask))
    return;

  

  // printf("gyro dma: %u, %u\n", s.dma_rx.mask, dma_hw->ints1);

  dma_hw->ints1 = s.dma_rx.mask;
  //printf("raw: ");print_vector3i16(&s.gyro.raw_rates);printf("\n");
  // cs_select(1);

  // if(s.completeCount++ % 1000 == 0)
  //   printf("g:dma:c\n");

  gyroUpdate();
}

static void __time_critical_func(gyro_data_ready_handler)(uint gpio, uint32_t events)
{
  if (gpio != s.gyro.config->dataReadyInterruptPin )
    return;
  
  gpio_acknowledge_irq(gpio, events);
  
  // if(s.startCount++ % 1000 == 0)
  //   printf("g:r, %u - %u\n", s.startCount, s.completeCount);
  //   return;

  //dma_start_channel_mask(s.dma_tx.mask | s.dma_rx.mask);

  

  // cs_select(0);
  dma_channel_hw_addr(s.dma_tx.id)->read_addr = (uintptr_t)s.gyroBuffer;
  dma_channel_hw_addr(s.dma_tx.id)->transfer_count = sizeof(s.gyroBuffer);

  dma_channel_hw_addr(s.dma_rx.id)->write_addr = (uintptr_t)&s.gyro.rx_reg;
  dma_channel_hw_addr(s.dma_rx.id)->transfer_count = sizeof(s.gyroBuffer);

  dma_start_channel_mask(s.dma_tx.mask | s.dma_rx.mask);
}

static void dma_init()
{
  // Grab some unused dma channels
  s.dma_tx.id = dma_claim_unused_channel(true);
  s.dma_tx.mask = 1u << s.dma_tx.id;

  s.dma_rx.id = dma_claim_unused_channel(true);
  s.dma_rx.mask = 1u << s.dma_rx.id;

  irq_add_shared_handler(DMA_IRQ_1, dma_complete_handler, PICO_HIGHEST_IRQ_PRIORITY);

  s.startCount = 0;
  s.completeCount = 0;

  uint spiId = spi_get_index(s.spi);

  printf("Configure TX DMA\n");
  dma_channel_config c = dma_channel_get_default_config(s.dma_tx.id);
  //channel_config_set_irq_quiet(&c, true);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_dreq(&c, spiId ? DREQ_SPI1_TX : DREQ_SPI0_TX);
  // channel_config_set_read_increment(&c, true);
  // channel_config_set_write_increment(&c, false);
  dma_channel_configure(
    s.dma_tx.id, 
    &c,
    &spi_get_hw(s.spi)->dr, // write address
    s.gyroBuffer,           // read address
    sizeof(s.gyroBuffer),     // element count (each element is of size transfer_data_size)
    false);                 // don't start yet

  printf("Configure RX DMA\n");



  // We set the inbound DMA to transfer from the SPI receive FIFO to a memory buffer paced by the SPI RX FIFO DREQ
  // We coinfigure the read address to remain unchanged for each element, but the write
  // address to increment (so data is written throughout the buffer)
  c = dma_channel_get_default_config(s.dma_rx.id);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_dreq(&c, spiId ? DREQ_SPI1_RX : DREQ_SPI0_RX);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, true);
  //channel_config_set_ring(&c, false, 4);

  dma_channel_configure(
    s.dma_rx.id, 
    &c,
    &s.gyro.rx_reg,         // write address
    &spi_get_hw(s.spi)->dr, // read address
    sizeof(s.gyroBuffer),   // element count (each element is of size transfer_data_size)
    false);                 // don't start yet

  //dma_start_channel_mask(s.dma_tx.mask | s.dma_rx.mask);

  printf("gyro: dma irg handler config\n");

  
  
  printf("gyro: dma irg handler config 2\n");
  dma_channel_set_irq1_enabled(s.dma_rx.id, true);

  printf("gyro: dma irg handler config 3\n");
  // if(!irq_is_enabled(DMA_IRQ_0))
  irq_set_enabled(DMA_IRQ_1, true); //this seems to be causing a panic

  //dma_start_channel_mask(s.dma_rx.mask);
  printf("gyro: dma init complete\n");
}

void gyroReadDev(spi_t spi, uint8_t *tx_data, uint8_t *rx_data, int8_t len)
{
  // cs_select(0);
  int bytes = spi_write_read_blocking(spi, tx_data, rx_data, len);
  // printf("%d, ", bytes);

  // for(int i=0; i < len; ++i)
  // {
  //   printf("%02X ", rx_data[i]);
  // }

  // printf("\n");
  // cs_select(1);
}

typedef struct
{
  union
  {
    struct
    {
      uint8_t reg;
      uint8_t data;
    } __packed;

    uint16_t value;
  };

} __packed GyroRegisterPacket_t;

uint8_t gyroReadRegister(uint8_t reg)
{
  GyroRegisterPacket_t tx = {reg | 0x80, 0x00};
  GyroRegisterPacket_t rx = {0, 0};

  printf("R:%02X|%02X->", tx.reg, tx.data);

  cs_select(0);
  int bytes = spi_write_read_blocking(s.spi, &tx.reg, &rx.reg, 2);
  cs_select(1);

  printf("%d, %02X%02X", bytes, rx.reg, rx.data);

  printf("\n");

  return rx.data;
}

void gyroWriteRegister(uint8_t reg, uint8_t data)
{
  GyroRegisterPacket_t tx = {reg, data};
  GyroRegisterPacket_t rx = {0, 0};

  printf("R:%02X|%02X->", tx.reg, tx.data);

  cs_select(0);
  int bytes = spi_write_read_blocking(s.spi, &tx.reg, &rx.reg, 2);
  cs_select(1);

  printf("%d, %02X%02X", bytes, rx.reg, rx.data);

  printf("\n");
}

void gyroUpdateState()
{
  int count = s.gyro.config->calibration.sampleCount;

  switch (s.gyro.state)
  {
    // boot up
  case GYRO_ST_BOOT:
    s.gyro.state = GYRO_ST_CAL_COLLECT;
    s.gyro.sampleCount = 0;
    break;

  // calibrate gyro
  case GYRO_ST_CAL_COLLECT: 
    if (s.gyro.sampleCount >= s.gyro.config->calibration.sampleCount)
    {
      s.gyro.state = GYRO_ST_CAL_COMPLETE;
      break;
    }

    for (int a = 0; a < 3; ++a)
    {
      s.gyro.cal.avg.axis[a] += ((float)s.gyro.raw_rates.axis[a]) / (float)count;
    }
    break;

  // process calibration data
  case GYRO_ST_CAL_COMPLETE:
  {
    uint8_t pass = 1;
    for (int a = 0; a < 3; ++a)
    {
      s.gyro.cal.zeroValue.axis[a] = (int16_t)s.gyro.cal.avg.axis[a];
    }

    if (!pass && s.gyro.cal.retry++ < s.gyro.config->calibration.maxRetries)
    {
      s.gyro.sampleCount = 0;
      s.gyro.cal.avg.roll = 0.0f;
      s.gyro.cal.avg.pitch = 0.0f;
      s.gyro.cal.avg.yaw = 0.0f;
      s.gyro.state = GYRO_ST_CAL_COLLECT;
      break;
    }


    s.gyro.state = pass ? GYRO_ST_READY : GYRO_ST_FAIL;
    break;
  }

  case GYRO_ST_READY:
    {
      s.gyro.raw_rates.roll -= (int32_t)s.gyro.cal.zeroValue.roll;
      s.gyro.raw_rates.pitch -= (int32_t)s.gyro.cal.zeroValue.pitch;
      s.gyro.raw_rates.yaw -= (int32_t)s.gyro.cal.zeroValue.yaw;
    // s.gyro.fixedRates[0].value = ((int32_t)s.gyro.raw_rates.roll) << 14;
    // s.gyro.fixedRates[1].value = ((int32_t)s.gyro.raw_rates.pitch) << 14;
    // s.gyro.fixedRates[2].value = ((int32_t)s.gyro.raw_rates.yaw) << 14;
    
      if(s.completeCallback)
        s.completeCallback();
    }
    break;

  case GYRO_ST_FAIL:
  default:
    break;
  }
}

void gyroUpdate()
{
  ++s.gyro.sampleCount;

  gyroUpdateState();
}

void gyroSetUpdateCallback(gyro_update_complete_callback_t cb)
{
  s.completeCallback = cb;
}

GyroState_t *gyroState()
{
  return &s.gyro;
}

void gyroConfigure()
{
  printf("gyro reset\n");
  gyroWriteRegister(MPU_RA_PWR_MGMT_1, MPU9250_BIT_RESET);

  sleep_ms(50);

  
  printf("gyro configure: %02X\n", gyroReadRegister(MPU_RA_WHO_AM_I));

  gyroWriteRegister(MPU_RA_PWR_MGMT_1, INV_CLK_PLL);
  gyroWriteRegister(MPU_RA_CONFIG, 0);//7);
  gyroWriteRegister(MPU_RA_GYRO_CONFIG, (INV_FSR_1000DPS << 3));
  gyroWriteRegister(MPU_RA_SMPLRT_DIV, 0);

  gyroWriteRegister(MPU_RA_ACCEL_CONFIG, INV_FSR_16G << 3);
  gyroWriteRegister(MPU_RA_INT_PIN_CFG, 0 << 7 | 0 << 6 | 0 << 5 | 1 << 4 | 0 << 3 | 0 << 2 | 1 << 1 | 0 << 0); // INT_ANYRD_2CLEAR, BYPASS_EN
  gyroWriteRegister(MPU_RA_INT_ENABLE, 0x01);

  spi_set_baudrate(s.spi, s.gyro.config->spi.clockSpeedGyro);
  printf("gyro configured\n");
}

void gyroInit(GyroConfig_t *info)
{
  memset(&s, 0, sizeof(s));
  s.gyro.state = GYRO_ST_BOOT;
  s.gyro.config = info;

  uint8_t readState[7] = {MPU_RA_GYRO_XOUT_H | 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  memcpy(s.gyroBuffer, readState, 7);

  printf("gyroInit\n");

  s.spi = spi0;

  spi_init(s.spi, s.gyro.config->spi.clockSpeedRegisters);

  gpio_set_function(s.gyro.config->spi.inputPin, GPIO_FUNC_SPI);
  gpio_set_function(s.gyro.config->spi.clockPin, GPIO_FUNC_SPI);
  gpio_set_function(s.gyro.config->spi.outputPin, GPIO_FUNC_SPI);
  gpio_set_function(s.gyro.config->spi.selectPin, GPIO_FUNC_SPI);

  gyroConfigure();
  
  // TODO: need to pump update state even if we don't get the interrupt
  

  dma_init();

  gpio_set_irq_enabled_with_callback(
    s.gyro.config->dataReadyInterruptPin, 
    GPIO_IRQ_EDGE_RISE, 
    true, 
    &gyro_data_ready_handler);

  gyroUpdateState();

  //dma_start_channel_mask(s.dma_tx.mask | s.dma_rx.mask);
}