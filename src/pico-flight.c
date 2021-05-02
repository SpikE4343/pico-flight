/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "flight_controller.h"

#define USING_PICO_PROBE 1

int main()
{
  stdio_init_all();


#if USING_PICO_PROBE
  gpio_set_function(16, GPIO_FUNC_UART);
  gpio_set_function(17, GPIO_FUNC_UART);
#endif

  // config.updateIntervals.gyro = 8000;      //8000.0;    //4000.0;
  // config.updateIntervals.control = 4000.0; //8000.0;
  // config.updateIntervals.print = 10;


  // config.osd.sync = 14;
  // config.osd.out = 15;

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


  // config.motorOutputs.dshotFreq = MOTOR_DSHOT600;
  // config.motorOutputs.min = MOTOR_MIN_OUTPUT;
  // config.motorOutputs.max = MOTOR_MAX_OUTPUT;
  // config.motorOutputs.idle = MOTOR_MIN_OUTPUT + MOTOR_OUTPUT_RANGE * 0.05f;
  // ;

  // config.telemetry.valueModBufferCount = 64;
  // config.telemetry.valueTableCount = 1024;

  // TODO: Need to move all data var registration calls to happen before full system 
  // initialization so that all values can be read from persistance

  printf("flightInit\n");
  flightInit();

  return 0;
}
