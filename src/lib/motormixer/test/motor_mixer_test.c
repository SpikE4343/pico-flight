#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include "basic_test.h"
#include "../src/motor_mixer.h"
MotorMixerConfig_t config;

void print_fixed_float(fixed_t v)
{
  fixed_print(v); printf(" :%f\n", fixed_to_float(v));
}

int setup_config()
{
  // simple quadcopter setup basically Betaflight X-Quad mixer defaults
  config.motorCount = 4;
  config.motor[0].mix[0] = fixed_from_float(-1.0f);
  config.motor[0].mix[1] = fixed_from_float(1.0f);
  config.motor[0].mix[2] = fixed_from_float(-1.0f);
  config.motor[0].mix[3] = fixed_from_float(1.0f);

  config.motor[1].mix[0] = fixed_from_float(-1.0f);
  config.motor[1].mix[1] = fixed_from_float(-1.0f);
  config.motor[1].mix[2] = fixed_from_float(1.0f);
  config.motor[1].mix[3] = fixed_from_float(1.0f);

  config.motor[2].mix[0] = fixed_from_float(1.0f);
  config.motor[2].mix[1] = fixed_from_float(1.0f);
  config.motor[2].mix[2] = fixed_from_float(1.0f);
  config.motor[2].mix[3] = fixed_from_float(1.0f);

  config.motor[3].mix[0] = fixed_from_float(1.0f);
  config.motor[3].mix[1] = fixed_from_float(-1.0f);
  config.motor[3].mix[2] = fixed_from_float(-1.0f);
  config.motor[3].mix[3] = fixed_from_float(1.0f);

  motorMixerInit(&config);
  return 1;
}

int test_simple_mixer()
{
  fixed_t inputs[5];
  fixed_t outputs[4];

  inputs[0] = fixed_from_float(1.0f); // roll
  inputs[1] = fixed_from_float(-1.0f); // pitch
  inputs[2] = fixed_from_float(-0.5f); // yaw
  inputs[3] = fixed_from_float(1.0f); // throttle
  inputs[4] = fixed_from_float(0.0f);

  motorMixerCalculateOutputs(inputs, outputs);

  print_fixed_float(outputs[0]);
  print_fixed_float(outputs[1]);
  print_fixed_float(outputs[2]);
  print_fixed_float(outputs[3]);


    

  return 1;
}

BT_SETUP();

int main()
{
  BT_BEGIN();
  setup_config();

  BT_ADD_TEST(test_simple_mixer);

  BT_END();
}