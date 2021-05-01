/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "flight_controller.h"

static FlightControllerConfig_t config;


int main()
{
  stdio_init_all();

  gpio_set_function(16, GPIO_FUNC_UART);
  gpio_set_function(17, GPIO_FUNC_UART);

  // TODO: ready config from flash file
  printf("!!!!!!!!!!!!! Invalid config loading defaults !!!!!!!!!!!!!!!!\n");
  uint8_t motorCount = 4;



  config.gyro.spi.inputPin = 0;
  config.gyro.spi.outputPin = 3;
  config.gyro.spi.clockPin = 2;
  config.gyro.spi.selectPin = 1;
  config.gyro.spi.clockSpeedRegisters = 8000000;
  config.gyro.spi.clockSpeedGyro = 20000000; //20000000;
  config.gyro.lpfCutoffHz = 50;
  config.gyro.scale = 1000 / (float)((65535.0/2.0)); //1 / 32.8f;//16.4f;
  config.gyro.dataReadyInterruptPin = 4;
  //config.updateIntervals.gyro = 32000.0; //4000.0;
  config.updateIntervals.gyro = 8000;      //8000.0;    //4000.0;
  config.updateIntervals.control = 4000.0; //8000.0;
  config.updateIntervals.print = 10;

  config.gyro.calibration.movementThreshold = 48 * 2;
  config.gyro.calibration.sampleCount = config.updateIntervals.gyro * 2;//config.updateIntervals.gyro * 1;
  config.gyro.calibration.maxRetries = 50;

  config.input.uart = 1;
  config.input.rxPin = 20;
  config.input.txPin = 21;

  config.osd.sync = 14;
  config.osd.out = 15;

  for (int a = 0; a < 3; ++a)
  {
    for (int v = 0; v < 4; ++v)
    {
      config.attitude.PID[a][v] = 1.0f;
    }
    config.attitude.PID[a][PID_I] = 0.0f;
  }

  config.input.controls.channel[INPUT_CONTROL_TROTTLE] = 0;
  config.input.controls.channel[INPUT_CONTROL_ROLL] = 1;
  config.input.controls.channel[INPUT_CONTROL_PITCH] = 2;
  config.input.controls.channel[INPUT_CONTROL_YAW] = 3;
  config.input.controls.channel[INPUT_CONTROL_ARM] = 6;

  config.motorOutputs.motorCount = motorCount;
  config.motorOutputs.startupDelayMs = 3000;
  config.motorOutputs.dshotFreq = MOTOR_DSHOT600;
  config.motorOutputs.min = MOTOR_MIN_OUTPUT;
  config.motorOutputs.max = MOTOR_MAX_OUTPUT;
  config.motorOutputs.idle = MOTOR_MIN_OUTPUT + MOTOR_OUTPUT_RANGE * 0.05f;
  ;

  config.motorOutputs.motor[0].pin = 6;
  config.motorOutputs.motor[1].pin = 7;
  config.motorOutputs.motor[2].pin = 8;
  config.motorOutputs.motor[3].pin = 9;
  // config.motorOutputs.motor[4].pin = 9;
  // config.motorOutputs.motor[5].pin = 10;
  // config.motorOutputs.motor[6].pin = 11;
  // config.motorOutputs.motor[7].pin = 12;

  config.mixer.motorCount = motorCount;
  config.mixer.motor[0].mix[0] =  -1.0f;
  config.mixer.motor[0].mix[1] =  1.0f;
  config.mixer.motor[0].mix[2] =  -1.0f;
  config.mixer.motor[0].mix[3] =  1.0f;

  config.mixer.motor[1].mix[0] =  -1.0f;
  config.mixer.motor[1].mix[1] =  -1.0f;
  config.mixer.motor[1].mix[2] =  1.0f;
  config.mixer.motor[1].mix[3] =  1.0f;

  config.mixer.motor[2].mix[0] =  1.0f;
  config.mixer.motor[2].mix[1] =  1.0f;
  config.mixer.motor[2].mix[2] =  1.0f;
  config.mixer.motor[2].mix[3] =  1.0f;

  config.mixer.motor[3].mix[0] =  1.0f;
  config.mixer.motor[3].mix[1] =  -1.0f;
  config.mixer.motor[3].mix[2] =  -1.0f;
  config.mixer.motor[3].mix[3] =  1.0f;

  config.telemetry.valueModBufferCount = 64;
  config.telemetry.valueTableCount = 1024;

  printf("flightInit\n");
  flightInit(&config);

  return 0;
}
