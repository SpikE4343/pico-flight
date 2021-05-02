#ifndef __motor_mixer_INCLUDED__
#define __motor_mixer_INCLUDED__

#include "telemetry.h"
#include "motor_common.h"

void motorMixerInit();
void motorMixerCalculateOutputs(TDataVar_t *input, TDataVar_t *output);

DECL_EXTERN_DV_ARRAY(tdv_motor0_mix);
DECL_EXTERN_DV_ARRAY(tdv_motor1_mix);
DECL_EXTERN_DV_ARRAY(tdv_motor2_mix);
DECL_EXTERN_DV_ARRAY(tdv_motor3_mix);

DECL_EXTERN_DV_ARRAY(tdv_motor_direction);

#endif