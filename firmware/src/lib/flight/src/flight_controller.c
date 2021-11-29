

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/float.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"
#include "web_server.h"

#include "filters.h"
#include "flight_controller.h"
#include "flight_attitude.h"
#include <sys/time.h>

#include "gyro_spi_mpu9250.h"
#include "motor_common.h"
#include "motor_mixer.h"
#include "motor_output.h"
#include "data_vars.h"
#include "camera.h"

#include "math.h"
#include "system.h"

#define FC_STATE_BOOT 0
#define FC_STATE_CALIBRATE 25
#define FC_STATE_CALIBRATE_FAIL 26
#define FC_STATE_DISARMED 100
#define FC_STATE_FAILSAFE 200
#define FC_STATE_ARMED 300



// ---------------------------------------------------------------
typedef struct
{
  float lpfAlpha;
  uint64_t startupMs;
  uint32_t gyroTicksPerControl;
} FlightControllerState_t;

static FlightControllerState_t state;

bool flightUpdate(repeating_timer_t* timer);
void flightPrintTask();

static repeating_timer_t timer;
static repeating_timer_t idle_timer;

static volatile uint32_t counter0 = 0;
static volatile uint32_t counter1 = 0;

// ---------------------------------------------------------------
void gyro_ready_callback(uint8_t gpio, uint32_t events)
{
  // Put the GPIO event(s) that just happened into event_str
  // so we can print it

  flightGyroUpdateTask(0);
}

// ---------------------------------------------------------------
bool flightIdleTimer(repeating_timer_t* timer)
{
  tdv_fc_core0_counter.v.u32 += ((int32_t)(counter0 - tdv_fc_core0_counter.v.u32)) >> 1;
  tdv_fc_core1_counter.v.u32 += ((int32_t)(counter1 - tdv_fc_core1_counter.v.u32)) >> 1;

  counter0 = 0;
  counter1 = 0;
}

// ---------------------------------------------------------------
void flightCore0()
{  
    // sample counters every millisecond
  if (!add_repeating_timer_us(-1000000.0 / 100.0, flightIdleTimer, NULL, &idle_timer))
  {
    printf("Failed to add timer\n");
    return;
  }

  // negative timeout means exact delay (rather than delay between callbacks)
  if (!add_repeating_timer_us(-1000000.0 / tdv_fc_telemetry_rate_hz.v.u32, flightUpdate, NULL, &timer))
  {
    printf("Failed to add timer\n");
    return;
  }

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  bool status = false;
  
  // webServerInit();

  while(true)
  {
    // webServerUpdate();
    if(++counter0 % 1000 == 0)
    {
      status = !status;
      gpio_put(PICO_DEFAULT_LED_PIN, status);
    }
  } 
}

// ---------------------------------------------------------------
void flightCore1()
{
  state.gyroTicksPerControl = (uint32_t)(tdv_gyro_sample_rate_hz.v.u32 / tdv_fc_update_rate_hz.v.u32);

  motorCommonInit();
  motorMixerInit();
  flightAttitudeInit();
  motorOutputInit();
  gyroInit();

  state.lpfAlpha = lpfAlpha(
      tdv_gyro_filter_hz.v.u32,
      1.0f / tdv_gyro_sample_rate_hz.v.u32);

  gyroSetUpdateCallback(flightGyroUpdateTask);

  multicore_fifo_push_blocking(1);

  while(true)
  {
    ++counter1;
  }
}

// ---------------------------------------------------------------
void flightInit()
{
  tdv_fc_state.v.i32 = FC_STATE_BOOT;

  multicore_launch_core1(flightCore1);

  // wait for control loop core init to complete
  int core1_init = multicore_fifo_pop_blocking();

  io_rc_init();
  osdInit();
  // camera_init();
  

  state.startupMs = system_time_us();

  flightCore0();
}

// ---------------------------------------------------------------
static inline float constrainf(float amt, float low, float high)
{
  if (amt < low)
    return low;

  if (amt > high)
    return high;

  return amt;
}

#define SETPOINT_RATE_LIMIT 1998.0f
#define RC_RATE_INCREMENTAL 14.54f

// ---------------------------------------------------------------
float applyBetaflightRates(const int axis, float rcCommandf, const float rcCommandfAbs)
{
  // ControlRates_t *rates = &config->control.rates[axis];

  // if (rates->expo)
  // {
  //   const float expof = rates->expo / 100.0f;
  //   rcCommandf = rcCommandf * (rcCommandfAbs * rcCommandfAbs * rcCommandfAbs) * expof + rcCommandf * (1 - expof);
  // }

  // float rcRate = rates->rc / 100.0f;
  // if (rcRate > 2.0f)
  //   rcRate += RC_RATE_INCREMENTAL * (rcRate - 2.0f);

  // float angleRate = 200.0f * rcRate * rcCommandf;
  // if (rates->super)
  // {
  //   const float rcSuperfactor = 1.0f / (constrainf(1.0f - (rcCommandfAbs * (rates->super / 100.0f)), 0.01f, 1.00f));
  //   angleRate *= rcSuperfactor;
  // }

  // return angleRate;
}

// ---------------------------------------------------------------
void flightProcessInputs()
{
  io_rc_update();

  tdv_fc_inputs[0].v.f32 = tdv_rc_input[tdv_rc_mapping[0].v.u8].v.f32;
  tdv_fc_inputs[1].v.f32 = tdv_rc_input[tdv_rc_mapping[1].v.u8].v.f32;
  tdv_fc_inputs[2].v.f32 = tdv_rc_input[tdv_rc_mapping[2].v.u8].v.f32;
  tdv_fc_inputs[3].v.f32 = tdv_rc_input[tdv_rc_mapping[3].v.u8].v.f32;
}

// ---------------------------------------------------------------
void flightControlUpdate()
{
  uint64_t now = (uint64_t)system_time_us();

  flightProcessInputs();

  uint32_t next = tdv_fc_state.v.u32;
  bool rxloss = ((uint32_t)now - tdv_rc_last_recv_us.v.u32)/1000 >= tdv_rc_recv_timeout.v.u32;
  tdv_rc_signal_lost.v.b8 = rxloss;

  bool outputEnabled = false;
  switch (tdv_fc_state.v.u32)
  {
    // ---------------------------------------------------------------
    case FC_STATE_BOOT:
      next = FC_STATE_CALIBRATE;
      break;

    // ---------------------------------------------------------------
    case FC_STATE_CALIBRATE:
    {
      if( tdv_gyro_state.v.u32 == GYRO_ST_READY)
        next = FC_STATE_DISARMED;
      else if( tdv_gyro_state.v.u32 == GYRO_ST_FAIL)
        next = FC_STATE_CALIBRATE_FAIL;
        
      break;
    }

    // ---------------------------------------------------------------
    case FC_STATE_ARMED:

      if (tdv_rc_failsafe.v.b8 )//|| 
          //tdv_rc_signal_lost.v.b8 )//||
          // rxloss)
      {
        
        tdv_fc_armed.v.b8 = false;

        next = FC_STATE_DISARMED;
        break;
      }

      if (!tdv_fc_armed.v.b8)
      {
        next = FC_STATE_DISARMED;
        break;
      }

      outputEnabled = tdv_motor_output_enabled.v.b8;
      flightAttitudeUpdate(
        tdv_fc_attitude_outputs, 
        tdv_fc_inputs, 
        tdv_fc_rates_filtered, 
        1.0f / tdv_fc_update_rate_hz.v.u32
        );
     
      break;

    // ---------------------------------------------------------------
    case FC_STATE_FAILSAFE:
      if (!bool8v(tdv_fc_armed) 
       && !bool8v(tdv_rc_failsafe) 
       ) //&& !bool8v(tdv_rc_signal_lost))
      {
        next = FC_STATE_DISARMED;
      }
      break;

    // ---------------------------------------------------------------
    case FC_STATE_DISARMED:
      tdv_fc_inputs[0].v.f32 = tdv_fc_attitude_outputs[0].v.f32 = 0.0f;
      tdv_fc_inputs[1].v.f32 = tdv_fc_attitude_outputs[1].v.f32 = 0.0f;
      tdv_fc_inputs[2].v.f32 = tdv_fc_attitude_outputs[2].v.f32 = 0.0f;
      tdv_fc_inputs[3].v.f32 = tdv_fc_attitude_outputs[3].v.f32 = 0.0f;

      tdv_fc_pidf_v_pitch[0].v.f32 = 0.0f;
      tdv_fc_pidf_v_pitch[1].v.f32 = 0.0f;
      tdv_fc_pidf_v_pitch[2].v.f32 = 0.0f;
      tdv_fc_pidf_v_pitch[3].v.f32 = 0.0f;

      tdv_fc_pidf_v_roll[0].v.f32 = 0.0f;
      tdv_fc_pidf_v_roll[1].v.f32 = 0.0f;
      tdv_fc_pidf_v_roll[2].v.f32 = 0.0f;
      tdv_fc_pidf_v_roll[3].v.f32 = 0.0f;

      tdv_fc_pidf_v_yaw[0].v.f32 = 0.0f;
      tdv_fc_pidf_v_yaw[1].v.f32 = 0.0f;
      tdv_fc_pidf_v_yaw[2].v.f32 = 0.0f;
      tdv_fc_pidf_v_yaw[3].v.f32 = 0.0f;

      for (int m = 0; m < tdv_motor_count.v.u8; ++m)
        tdv_motor_output[m].v.f32 = 0;

      if  (bool8v(tdv_fc_armed) 
       && !bool8v(tdv_rc_failsafe) 
      //  && !bool8v(tdv_rc_signal_lost)
      )//  && !rxloss)
      {
        next = FC_STATE_ARMED;
        break;
      }
      break;

    // ---------------------------------------------------------------
    default:
    {
      if (now < (state.startupMs + (uint64_t)tdv_motor_startup_delay_ms.v.u32))
      {
        //++state.controlUpdates;
      }
      else if (now < (state.startupMs + tdv_motor_startup_delay_ms.v.u32 * 2))
      {
        for (int m = 0; m < tdv_motor_count.v.u8; ++m)
          tdv_motor_output[m].v.f32 = 0;
      }
      else
      {
        next = FC_STATE_DISARMED;
      }
    }
    break;
  }

  if (next != tdv_fc_state.v.u32)
  {
    tdv_fc_state.v.u32 = next;
    telemetry_sample_var(&tdv_fc_state);
  }

  motorMixerCalculateOutputs(tdv_fc_attitude_outputs, tdv_motor_output, tdv_motor_count.v.u8);
  motorOutputSet(outputEnabled, tdv_motor_output);

  ++tdv_fc_control_updates.v.u32;
}

#define GYRO_SCALE (1000.0f / (float)(65535.0f/2.0f))

// ---------------------------------------------------------------
void flightGyroUpdateTask()
{
  uint32_t t = timer_hw->timelr;
  GyroState_t* gyro = gyroState();

  tdv_fc_rates_raw[0].v.f32 = ((float)gyro->raw_rates.axis[0]) / 32.8f / 1000.0f; //* GYRO_SCALE;
  tdv_fc_rates_raw[1].v.f32 = ((float)gyro->raw_rates.axis[1]) / 32.8f / 1000.0f; //* GYRO_SCALE;
  tdv_fc_rates_raw[2].v.f32 = ((float)gyro->raw_rates.axis[2]) / 32.8f / 1000.0f; //* GYRO_SCALE;
  
  // TODO: explore using hardware interp in blend mode as a low pass filter

  tdv_fc_rates_filtered[0].v.f32 = lowPassFilter(tdv_fc_rates_filtered[0].v.f32, tdv_fc_rates_raw[0].v.f32, state.lpfAlpha);
  tdv_fc_rates_filtered[1].v.f32 = lowPassFilter(tdv_fc_rates_filtered[1].v.f32, tdv_fc_rates_raw[1].v.f32, state.lpfAlpha);
  tdv_fc_rates_filtered[2].v.f32 = lowPassFilter(tdv_fc_rates_filtered[2].v.f32, tdv_fc_rates_raw[2].v.f32, state.lpfAlpha);

  ++tdv_fc_gyro_updates.v.u32;

  if(tdv_fc_gyro_updates.v.u32 % state.gyroTicksPerControl == 0 )
  {
    flightControlUpdate();
  }

  int32_t dt = timer_hw->timelr - t;
  tdv_fc_gyro_update_us.v.u32 += ((int32_t)(dt - tdv_fc_gyro_update_us.v.u32)) >> 1;
  //return true;
}

// ---------------------------------------------------------------
bool flightUpdate(repeating_timer_t* timer)
{
  flightPrintTask();
  return true;
}


static int x = 0;
static int y = 0;
static char buffer[64];
char a = 0;
double temp = 0.0f;

// ---------------------------------------------------------------
void flightPrintTask()
{
  uint64_t time = system_time_us();

  // telemetry_sample_array(tdv_fc_rates_raw, 3);
  telemetry_sample_var_array(tdv_fc_rates_filtered, 3);
  telemetry_sample_var_array(tdv_motor_output, tdv_motor_count.v.u32);
  telemetry_sample_var_array(tdv_motor_out_cmd, tdv_motor_count.v.u32);

  //telemetry_sample_var_array(tdv_motor_output, tdv_motor_count.v.u8);

  telemetry_sample_var(&tdv_fc_armed);
  // telemetry_sample_var(&tdv_fc_gyro_updates);
  telemetry_sample_var(&tdv_fc_state);
  // telemetry_sample_var(&tdv_fc_control_updates);

  telemetry_sample_var(&tdv_fc_core0_counter);
  telemetry_sample_var(&tdv_fc_core1_counter);

  telemetry_sample_var(&tdv_fc_gyro_update_us);

  telemetry_sample_var(&tdv_rc_uart_rx_bytes);

  // telemetry_sample_var_array(tdv_fc_attitude_outputs, 4);
  telemetry_sample_var_array(tdv_rc_input, 8);
  telemetry_sample_var(&tdv_rc_rssi);
  telemetry_sample_var(&tdv_rc_packet_loss);
  telemetry_sample_var(&tdv_rc_frames_recv);

  tdv_rc_packet_loss.v.u32 = 0;
  tdv_rc_frames_recv.v.u32 = 0;

  // telemetry_sample_var_array(&tdv_fc_pidf_v_roll, 4);
  // telemetry_sample_var_array(&tdv_fc_pidf_v_pitch, 4);
  // telemetry_sample_var_array(&tdv_fc_pidf_v_yaw, 4);

  telemetry_sample_var_array(tdv_fc_pid_sum, 3);
  // telemetry_sample_var_array(&tdv_fc_pid_sp, 3);
  // telemetry_sample_var_array(&tdv_fc_pid_sp_error, 3);
  // telemetry_sample_var_array(&tdv_fc_pid_sp_delta, 3);
  // telemetry_sample_var_array(&tdv_fc_pid_pv_error, 3);
  // telemetry_sample_var_array(&tdv_fc_pid_pv, 3);

  // telemetry_sample_var(&tdv_rc_uart_rx_bytes);
  // telemetry_sample_var(&tdv_rc_uart_tx_bytes);
  
  // sprintf(buffer, "% u    ", tdv_fc_core0_counter.v.u32);
  // osdDrawString(10, 90, buffer);

  tdv_fc_core0_counter.v.u32 = 0;
  tdv_fc_core1_counter.v.u32 = 0;

  tdv_rc_uart_rx_bytes.v.u32 = 0;
  tdv_rc_uart_tx_bytes.v.u32 = 0;

  
  telemetry_send(state.startupMs, time);
  telemetry_update();

  //osdDraw( 40 + (x++ * 8) % 100, (x / 100)+25,1);
  // osdDraw( (x) % 515, 11, (x % 4) > 0);
  // ++x;

  // //printf("buffer=%s\n",buffer);
  // temp = tdv_fc_gyro_roll.value.f32;
  // sprintf(buffer, "%f    ",temp);
  // osdDrawString(10, 80, buffer);

 

  // sprintf(buffer, "%f    ", tdv_fc_gyro_yaw.value.f32);
  // osdDrawString(10, 100, buffer);
  
  //osdDrawString2(40 + (x += 16) % 200, ((x / 200)*16)%200 +25, buffer);
  
  state.lpfAlpha = lpfAlpha(
      tdv_gyro_filter_hz.v.u32,
      tdv_gyro_sample_rate_hz.v.u32);

  state.gyroTicksPerControl = (uint32_t)(tdv_gyro_sample_rate_hz.v.u32 / tdv_fc_update_rate_hz.v.u32);
}
