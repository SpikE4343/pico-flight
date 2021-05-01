#ifndef __motor_mixer_INCLUDED__
#define __motor_mixer_INCLUDED__

#include "vector.h"
// #include "math_fixed.h"

typedef struct
{
  float mix[4];
  uint8_t direction;
} MotorMixConfig_t;

typedef struct
{
  uint8_t motorCount;
  MotorMixConfig_t motor[4];
} MotorMixerConfig_t;

void motorMixerInit(MotorMixerConfig_t *info);
void motorMixerCalculateOutputs(float *input, float *output);

#endif