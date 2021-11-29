

#include <stdio.h>
#include <stdlib.h>
#include "data_vars.h"

// ---------------------------------------------------------------
void gyro_vars_init()
{
  telemetry_register_var(&tdv_gyro_spi_pins_in);
  telemetry_register_var(&tdv_gyro_spi_pins_out);
  telemetry_register_var(&tdv_gyro_spi_pins_clk);
  telemetry_register_var(&tdv_gyro_spi_pins_select);

  telemetry_register_var(&tdv_gyro_spi_clk_reg_hz);
  telemetry_register_var(&tdv_gyro_spi_clk_rates_hz);

  telemetry_register_var(&tdv_gyro_sample_rate_hz);
  telemetry_register_var(&tdv_gyro_filter_hz);
  telemetry_register_var(&tdv_gyro_rate_scale);
  telemetry_register_var(&tdv_gyro_rdy_pin);

  telemetry_register_var(&tdv_gyro_sample_count);
  telemetry_register_var(&tdv_gyro_state);

  telemetry_register_var(&tdv_gyro_cal_samples);
  telemetry_register_var(&tdv_gyro_cal_retries);
}

// ---------------------------------------------------------------
void fc_vars_init()
{
  telemetry_register_var(&tdv_fc_state);
  telemetry_register_var(&tdv_fc_gyro_updates);

  telemetry_register_var(&tdv_fc_control_updates);

  telemetry_register_var(&tdv_fc_core0_counter);
  telemetry_register_var(&tdv_fc_core1_counter);
  telemetry_register_var(&tdv_fc_gyro_update_us);  

  telemetry_register_var(&tdv_fc_armed);  

  telemetry_register_var(&tdv_fc_update_rate_hz);
  telemetry_register_var(&tdv_fc_telemetry_rate_hz);
  telemetry_register_var(&tdv_fc_failsafe_rx_loss_ms);
  

  telemetry_register_var_array(tdv_fc_rates_raw, 3);
  telemetry_register_var_array(tdv_fc_rates_filtered, 3);

  telemetry_register_var_array(tdv_fc_inputs, 4);
  telemetry_register_var_array(tdv_fc_attitude_outputs, 4);

  telemetry_register_var_array(tdv_fc_pidf_k_roll, 4);
  telemetry_register_var_array(tdv_fc_pidf_k_pitch, 4);
  telemetry_register_var_array(tdv_fc_pidf_k_yaw, 4);

  telemetry_register_var_array(tdv_fc_pidf_v_roll, 4);
  telemetry_register_var_array(tdv_fc_pidf_v_pitch, 4);
  telemetry_register_var_array(tdv_fc_pidf_v_yaw, 4);


  telemetry_register_var_array(tdv_fc_pid_sum, 3);
  telemetry_register_var_array(tdv_fc_pid_sp, 3);
  telemetry_register_var_array(tdv_fc_pid_sp_error, 3);

  telemetry_register_var_array(tdv_fc_pid_sp_delta, 3);
  telemetry_register_var_array(tdv_fc_pid_pv_error, 3);
  telemetry_register_var_array(tdv_fc_pid_pv, 3);
}

// ---------------------------------------------------------------
void motor_common_init()
{
  telemetry_register_var_array(tdv_motor_direction, 4);
  telemetry_register_var(&tdv_motor_count);
}

// ---------------------------------------------------------------
void motor_mixer_init()
{
  telemetry_register_var_array(tdv_motor0_mix, 4);
  telemetry_register_var_array(tdv_motor1_mix, 4);
  telemetry_register_var_array(tdv_motor2_mix, 4);
  telemetry_register_var_array(tdv_motor3_mix, 4);

  telemetry_register_var_array( tdv_mixer_input, 4 );
}

// ---------------------------------------------------------------
void motor_output_init()
{
  telemetry_register_var_array(tdv_motor_output, 4);
  telemetry_register_var_array(tdv_motor_output_pin, 4);
  telemetry_register_var_array(tdv_motor_out_cmd, 4);

  telemetry_register_var(&tdv_motor_startup_delay_ms);
  telemetry_register_var(&tdv_motor_output_min);
  telemetry_register_var(&tdv_motor_output_idle);
  telemetry_register_var(&tdv_motor_output_max);
  telemetry_register_var(&tdv_motor_output_enabled);
  telemetry_register_var(&tdv_motor_output_rate);
}

// ---------------------------------------------------------------
void rc_vars_init()
{
  telemetry_register_var(&tdv_rc_signal_lost);
  telemetry_register_var(&tdv_rc_failsafe);
  telemetry_register_var(&tdv_rc_frames_recv);

  telemetry_register_var(&tdv_rc_uart_pins_tx);
  telemetry_register_var(&tdv_rc_uart_pins_rx);
  telemetry_register_var(&tdv_rc_uart_baud);
  telemetry_register_var(&tdv_rc_uart_id);
  
  telemetry_register_var(&tdv_rc_rssi);
  telemetry_register_var(&tdv_rc_packet_loss);
  telemetry_register_var(&tdv_rc_recv_state);
  
  telemetry_register_var(&tdv_rc_uart_rx_bytes);
  telemetry_register_var(&tdv_rc_uart_tx_bytes);

  telemetry_register_var(&tdv_rc_last_recv_us);
  telemetry_register_var(&tdv_rc_recv_timeout);
  
  

  telemetry_register_var_array(tdv_rc_mapping, 8);
  telemetry_register_var_array(tdv_rc_input, 8);
}

// ---------------------------------------------------------------
void data_vars_init()
{
  printf("init data vars system\n");
  gyro_vars_init();
  fc_vars_init();
  motor_common_init();
  motor_mixer_init();
  motor_output_init();
  rc_vars_init();

}

