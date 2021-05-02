#ifndef __motor_mixer_INCLUDED__
#define __motor_mixer_INCLUDED__

#include "vector.h"
#include "telemetry.h"

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
void motorMixerCalculateOutputs(TDataVar_t *input, TDataVar_t *output);

DECL_EXTERN_DV_ARRAY(tdv_motor0_mix)
DECL_EXTERN_DV_ARRAY(tdv_motor1_mix)
DECL_EXTERN_DV_ARRAY(tdv_motor2_mix)
DECL_EXTERN_DV_ARRAY(tdv_motor3_mix)

#endif