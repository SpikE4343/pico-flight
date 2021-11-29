#ifndef gyro_vars_INCLUDED
#define gyro_vars_INCLUDED

#include "telemetry_data.h"

DECL_EXTERN_DATA_VAR(tdv_gyro_spi_pins_in);     //
DECL_EXTERN_DATA_VAR(tdv_gyro_spi_pins_out);    //
DECL_EXTERN_DATA_VAR(tdv_gyro_spi_pins_clk);    //
DECL_EXTERN_DATA_VAR(tdv_gyro_spi_pins_select); //

DECL_EXTERN_DATA_VAR(tdv_gyro_spi_clk_reg_hz);   //
DECL_EXTERN_DATA_VAR(tdv_gyro_spi_clk_rates_hz); //

DECL_EXTERN_DATA_VAR(tdv_gyro_filter_hz);  //
DECL_EXTERN_DATA_VAR(tdv_gyro_rate_scale); //
DECL_EXTERN_DATA_VAR(tdv_gyro_rdy_pin);    //

DECL_EXTERN_DATA_VAR(tdv_gyro_sample_count); //
DECL_EXTERN_DATA_VAR(tdv_gyro_state);   
DECL_EXTERN_DATA_VAR(tdv_gyro_sample_rate_hz);
     //
DECL_EXTERN_DATA_VAR(tdv_gyro_cal_samples);  //
DECL_EXTERN_DATA_VAR(tdv_gyro_cal_retries);  //

#endif