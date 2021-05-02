
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "math_util.h"

#include "motor_mixer.h"


void motorMixerInit(MotorMixerConfig_t *_config)
{
  printf("mixer init\n");
  config = _config;
}

int motorHasValue(float val)
{
  return (abs(val) >= 0.2f);
}


void motorMixerCalculateOutputs(TDataVar_t *input, TDataVar_t *output)
{
  int m = 0, c = 0;
  MotorMixConfig_t *mmix = NULL;
  float tempOut[4];
  float tr = 0.0f, tp = 0.0f, ty = 0.0f, tt = 0.0f;
  float temp = 1.0f, min = 2.0f, max = -2.0f, scaled = 0.0f;

  for (m = 0; m < config->motorCount; ++m)
  {
    mmix = &config->motor[m];
    tr = input[0] * mmix->mix[0];
    tp = input[1] * mmix->mix[1];
    ty = input[2] * mmix->mix[2];
    tt = input[3] * mmix->mix[3];

    temp = (tr+tp)+ ty;
  
    tempOut[m] = temp;

    max = fmax(max, tempOut[m]);
    min = fmin(min, tempOut[m]);
  }

  float range = (max - min);
  

  // only normalize if we are saturating atleast one motor
  range = fmax(range, 1.0);

  float normMin = min /range;
  float normMax = max / range;

  for (m = 0; m < config->motorCount; ++m)
  {
    mmix = &config->motor[m];
    
    output[m] = (tempOut[m] / range);
    // tempOut[m] = fixed_add(tempOut[m], normMax);
    // output[m] = fixed_sub( tempOut[m], normMin);
  }
}

#define motor0_mapping_name "motor0.mix"
#define motor1_mapping_name "motor1.mix"
#define motor2_mapping_name "motor2.mix"
#define motor3_mapping_name "motor3.mix"
#define rc_mapping_desc "RC input mapping to controls"

BEGIN_DEF_DV_ARRAY( tdv_motor0_mix )
  DEF_DV_ARRAY_ITEM(0, -1.0f, motor0_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(1, 1.0f, motor0_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(2, -1.0f, motor0_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(3, 1.0f, motor0_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
END_DEF_DV_ARRAY()

BEGIN_DEF_DV_ARRAY( tdv_motor1_mix )
  DEF_DV_ARRAY_ITEM(0, -1.0f, motor1_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(1, -1.0f, motor1_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(2, 1.0f, motor1_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(3, 1.0f, motor1_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
END_DEF_DV_ARRAY()

BEGIN_DEF_DV_ARRAY( tdv_motor2_mix )
  DEF_DV_ARRAY_ITEM(0, 1.0f, motor2_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(1, 1.0f, motor2_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(2, 1.0f, motor2_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(3, 1.0f, motor2_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
END_DEF_DV_ARRAY()

BEGIN_DEF_DV_ARRAY( tdv_motor3_mix )
  DEF_DV_ARRAY_ITEM(0, 1.0f, motor3_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(1, -1.0f, motor3_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(2, -1.0f, motor3_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
  DEF_DV_ARRAY_ITEM(3, 1.0f, motor3_mapping_name, rc_mapping_desc, Tdt_f32, Tdm_RW)
END_DEF_DV_ARRAY()