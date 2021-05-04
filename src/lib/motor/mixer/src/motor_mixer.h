#ifndef __motor_mixer_INCLUDED__
#define __motor_mixer_INCLUDED__

#include "telemetry.h"
#include "motor_common.h"

void motorMixerInit();
void motorMixerCalculateOutputs(TDataVar_t *input, TDataVar_t *output);


#endif