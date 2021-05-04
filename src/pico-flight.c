/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "flight_controller.h"
#include "data_vars.h"

#define USING_PICO_PROBE 1

int main()
{
  stdio_init_all();


#if USING_PICO_PROBE
  gpio_set_function(16, GPIO_FUNC_UART);
  gpio_set_function(17, GPIO_FUNC_UART);
#endif

  // for (int a = 0; a < 3; ++a)
  // {
  //   for (int v = 0; v < 4; ++v)
  //   {
  //     config.attitude.PID[a][v] = 1.0f;
  //   }
  //   config.attitude.PID[a][PID_I] = 0.0f;
  // }

  // config.input.controls.channel[INPUT_CONTROL_TROTTLE] = 0;
  // config.input.controls.channel[INPUT_CONTROL_ROLL] = 1;
  // config.input.controls.channel[INPUT_CONTROL_PITCH] = 2;
  // config.input.controls.channel[INPUT_CONTROL_YAW] = 3;
  // config.input.controls.channel[INPUT_CONTROL_ARM] = 6;



  telemetryInit();
  data_vars_init();


  flightInit();

  return 0;
}
