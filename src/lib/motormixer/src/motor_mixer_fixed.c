
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "math_util.h"

#include "motor_mixer.h"

typedef struct
{

} MotorMixerState_t;

static MotorMixerConfig_t *config;
static MotorMixerState_t state;

void motorMixerInit(MotorMixerConfig_t *_config)
{
  printf("mixer init\n");
  config = _config;
}

int motorHasValue(float val)
{
  return (abs(val) >= 0.2f);
}


#define print_fixed_named(v) print_fixed(v, #v);

void print_fixed(fixed_t v, char* name)
{
  fixed_print(v); printf(":%s %f\n", name, fixed_to_float(v));
}

void motorMixerCalculateOutputs(fixed_t *input, fixed_t *output)
{
  int m = 0, c = 0;
  MotorMixConfig_t *mmix = NULL;
  fixed_t tempOut[4];
  fixed_t tr = fixed_from_float(0.0f), tp = fixed_from_float(0.0f), ty = fixed_from_float(0.0f), tt = fixed_from_float(0.0f);
  fixed_t temp = fixed_from_float(1.0f), min = fixed_from_float(2.0f), max = fixed_from_float(-2.0f), scaled = fixed_from_float(0.0f);

  for (m = 0; m < config->motorCount; ++m)
  {
    mmix = &config->motor[m];
    tr = fixed_mul(input[0],mmix->mix[0]);
    tp = fixed_mul(input[1],mmix->mix[1]);
    ty = fixed_mul(input[2],mmix->mix[2]);
    tt = fixed_mul(input[3],mmix->mix[3]);

    print_fixed_named(tr);
    print_fixed_named(tp);
    print_fixed_named(ty);
    print_fixed_named(tt);
    

    
    // c = motorHasValue(tr);
    // c += motorHasValue(tp);
    // c += motorHasValue(ty);
    //c += motorHasValue(tt);
    temp = fixed_add(fixed_add(tr,tp), ty);//fixed_add(ty, tt));
     
  
    //fixed_print(temp); printf(":temp %f\n", fixed_to_float(temp));

    // handle saturation only when exceeding the 
    // normalized range of -1 to 1
    // 
    
    // if (c > 0)
    // {
    //   temp /= c;
    // }

    tempOut[m] = temp;
    print_fixed_named(temp);

    //printf("\nm[%u]: %f, c: %d, temp: %f ", m, tempOut[m], c, temp);

    max = fixed_max(max, tempOut[m]);
    print_fixed_named(max);
    min = fixed_min(min, tempOut[m]);
    print_fixed_named(min);
  }

  fixed_t range = fixed_sub(max, min);
  

  // only normalize if we are saturating atleast one motor
  range = fixed_max(range, fixed_from_int(1));

  fixed_t normMin = fixed_div(min, range);
  fixed_t normMax = fixed_div(max, range);

  for (m = 0; m < config->motorCount; ++m)
  {
    mmix = &config->motor[m];
    
    output[m] = fixed_div(tempOut[m], range);
    // tempOut[m] = fixed_add(tempOut[m], normMax);
    // output[m] = fixed_sub( tempOut[m], normMin);
  }
}