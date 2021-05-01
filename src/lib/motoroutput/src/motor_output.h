#ifndef __motor_output_h_INCLUDED__
#define __motor_output_h_INCLUDED__

#include <stdint.h>
#include "math_fixed.h"

#define MAX_MOTORS 4

typedef struct
{
  uint8_t pin;
} SingleMotorOutputConfig_t;

typedef struct
{
  uint8_t motorCount;
  uint16_t startupDelayMs;
  uint16_t min;
  uint16_t max;
  uint16_t idle;
  uint16_t dshotFreq;
  SingleMotorOutputConfig_t motor[MAX_MOTORS];
} MotorOutputConfig_t;

// inputs from mixer
typedef struct
{
  int disarmed;
  uint8_t motorCount;
  fixed_t motor[MAX_MOTORS]; // 0.0-1.0f
} MotorOutputs_t;

// output values to esc/motors
typedef struct
{
  uint8_t motorCount;
  uint16_t motor[MAX_MOTORS];
} MotorOutputValues_t;

void motorOutputInit(MotorOutputConfig_t *info);
void motorOutputSet(MotorOutputs_t *output);

#endif