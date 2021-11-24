

#include "data_vars.h"

// --------------------- Runtime vars --------------------- 
DEF_DATA_VAR(tdv_gyro_state, 0, 
  "gyro.state",
  "Gyroscope sensor controller state",
  u8, Tdm_read);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_sample_count, 0, 
  "gyro.samples",
  "Number of samples read from gyro sensor",
  u32, Tdm_read);

// ------------------- Configuration Vars ---------------------
DEF_DATA_VAR(tdv_gyro_filter_hz, 50, 
  "gyro.filter.hz",
  "Cutoff frequency of gyro filter",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_rate_scale, 1000 / (float)((65535.0/2.0)), 
  "gyro.rate.scale",
  "Max angular rotation rate in deg/s",
  f32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_rdy_pin, 4, 
  "gyro.data_ready.pin",
  "Pin where gyro will raise \"data ready\" signal",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_spi_pins_in, 0, 
  "gyro.spi.pins.in",
  "Gyro SPI data input gpio pin",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_spi_pins_out, 3, 
  "gyro.spi.pins.out",
  "Gyro SPI data ouput gpio pin",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_spi_pins_clk, 2, 
  "gyro.spi.pins.clock",
  "Gyro SPI data clock gpio pin",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_spi_pins_select, 1, 
  "gyro.spi.pins.select",
  "Gyro SPI chip select gpio pin",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_spi_clk_reg_hz, 8000000, 
  "gyro.spi.clock.registers",
  "Gyro SPI clock speed for reading/writing registers",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_spi_clk_rates_hz, 20000000, 
  "gyro.spi.clock.rates",
  "Gyro SPI clock speed for reading gyro rates",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_cal_samples, 8000 * 2, 
  "gyro.calibration.samples",
  "Number of raw gyro samples to average to calculate offests",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_cal_retries, 10, 
  "gyro.calibration.retries",
  "Number attempts to calculate gyro offests",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_gyro_sample_rate_hz, 8000, 
  "gyro.sample_rate",
  "Gyro samples rate to read, in hertz",
  u32, Tdm_RW | Tdm_config);



