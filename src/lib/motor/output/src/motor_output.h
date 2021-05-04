#ifndef __motor_output_h_INCLUDED__
#define __motor_output_h_INCLUDED__

#include <stdint.h>
#include "motor_common.h"
#include "telemetry.h"





// inputs from mixer


// output values to esc/motors
typedef struct
{
  uint8_t motorCount;
  uint16_t motor[MAX_MOTORS];
} MotorOutputValues_t;

void motorOutputInit();
void motorOutputSet(TDataVar_t *output);


#endif