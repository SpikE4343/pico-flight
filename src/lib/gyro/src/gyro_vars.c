

#include "gyro_vars.h"
#include "telemetry.h"



// --------------------- Runtime vars --------------------- 
DEF_DATA_VAR(tdv_gyro_state, 0, 
  "gyro.state",
  "Gyroscope sensor controller state",
  Tdt_u8, Tdm_read);

DEF_DATA_VAR(tdv_gyro_sample_count, 0, 
  "gyro.samples",
  "Number of samples read from gyro sensor",
  Tdt_u32, Tdm_read);

// ------------------- Configuration Vars ---------------------
DEF_DATA_VAR(tdv_gyro_filter_hz, 0, 
  "gyro.filter.hz",
  "Cutoff frequency of gyro filter",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_rate_scale, 1000, 
  "gyro.rate.scale",
  "Max angular rotation rate in deg/s",
  Tdt_u32, Tdm_RW | Tdm_config);


DEF_DATA_VAR(tdv_gyro_rdy_pin, 0, 
  "gyro.pins.rdy",
  "Pin where gyro will raise \"data ready\" signal",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_spi_pins_in, 0, 
  "gyro.spi.pins.in",
  "Gyro SPI data input gpio pin",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_spi_pins_out, 0, 
  "gyro.spi.pins.out",
  "Gyro SPI data ouput gpio pin",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_spi_pins_clk, 0, 
  "gyro.spi.pins.clock",
  "Gyro SPI data clock gpio pin",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_spi_pins_select, 0, 
  "gyro.spi.pins.select",
  "Gyro SPI chip select gpio pin",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_spi_clk_reg, 0, 
  "gyro.spi.clock.registers",
  "Gyro SPI clock speed for reading/writing registers",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_spi_clk_rates, 0, 
  "gyro.spi.clock.rates",
  "Gyro SPI clock speed for reading gyro rates",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_cal_samples, 8000 * 2, 
  "gyro.calibration.samples",
  "Number of raw gyro samples to average to calculate offests",
  Tdt_u32, Tdm_RW | Tdm_config);

DEF_DATA_VAR(tdv_gyro_cal_retries, 10, 
  "gyro.calibration.retries",
  "Number attempts to calculate gyro offests",
  Tdt_u32, Tdm_RW | Tdm_config);

void gyroInitVars()
{
  telemetry_register(&tdv_gyro_spi_pins_in);
  telemetry_register(&tdv_gyro_spi_pins_out);
  telemetry_register(&tdv_gyro_spi_pins_clock);
  telemetry_register(&tdv_gyro_spi_pins_select);

  telemetry_register(&tdv_gyro_spi_clk_reg);
  telemetry_register(&tdv_gyro_spi_clk_rates);

  telemetry_register(&tdv_gyro_filter_hz);
  telemetry_register(&tdv_gyro_rate_scale);
  telemetry_register(&tdv_gyro_rdy_pin);

  telemetry_register(&tdv_gyro_sample_count);
  telemetry_register(&tdv_gyro_state);

  telemetry_register(&tdv_gyro_cal_samples);
  telemetry_register(&tdv_gyro_cal_retries);
}

