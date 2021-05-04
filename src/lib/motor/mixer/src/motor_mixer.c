
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "math_util.h"
#include "data_vars.h"

#include "motor_mixer.h"


TDataVar_t* mmixes[] = {
  &tdv_motor0_mix[0],
  &tdv_motor1_mix[0],
  &tdv_motor2_mix[0],
  &tdv_motor3_mix[0]
};

void motorMixerInit()
{
  printf("mixer init\n");

}

int motorHasValue(float val)
{
  return (abs(val) >= 0.2f);
}


void motorMixerCalculateOutputs(TDataVar_t *input, TDataVar_t *output)
{
  int m = 0, c = 0;
  float tempOut[4];
  float tr = 0.0f, tp = 0.0f, ty = 0.0f, tt = 0.0f;
  float temp = 1.0f, min = 2.0f, max = -2.0f, scaled = 0.0f;

  for (m = 0; m < tdv_motor_count.v.u8; ++m)
  {
    TDataVar_t* mix = mmixes[m];
    tr = input[0].v.f32 * mix[0].v.f32;
    tp = input[1].v.f32 * mix[1].v.f32;
    ty = input[2].v.f32 * mix[2].v.f32;
    tt = input[3].v.f32 * mix[3].v.f32;

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

  for (m = 0; m < tdv_motor_count.v.u8; ++m)
  {
    TDataVar_t* mix = mmixes[m];
    
    output[m].v.f32 = (tempOut[m] / range);
    // tempOut[m] = fixed_add(tempOut[m], normMax);
    // output[m] = fixed_sub( tempOut[m], normMin);
  }
}

#define motor0_mapping_name "motor.0.mix"
#define motor1_mapping_name "motor.1.mix"
#define motor2_mapping_name "motor.2.mix"
#define motor3_mapping_name "motor.3.mix"
#define motor_mapping_desc "Motor mix values for each axis"

BEGIN_DEF_DV_ARRAY( tdv_motor0_mix )
  DEF_DV_ARRAY_ITEM(0, -1.0f, motor0_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(1, 1.0f, motor0_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(2, -1.0f, motor0_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(3, 1.0f, motor0_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config)
END_DEF_DV_ARRAY();

BEGIN_DEF_DV_ARRAY( tdv_motor1_mix )
  DEF_DV_ARRAY_ITEM(0, -1.0f, motor1_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(1, -1.0f, motor1_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(2, 1.0f, motor1_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(3, 1.0f, motor1_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config)
END_DEF_DV_ARRAY();

BEGIN_DEF_DV_ARRAY( tdv_motor2_mix )
  DEF_DV_ARRAY_ITEM(0, 1.0f, motor2_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(1, 1.0f, motor2_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(2, 1.0f, motor2_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(3, 1.0f, motor2_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
END_DEF_DV_ARRAY();

BEGIN_DEF_DV_ARRAY( tdv_motor3_mix )
  DEF_DV_ARRAY_ITEM(0, 1.0f, motor3_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(1, -1.0f, motor3_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(2, -1.0f, motor3_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM(3, 1.0f, motor3_mapping_name, motor_mapping_desc, f32, Tdm_RW | Tdm_config),
END_DEF_DV_ARRAY();

