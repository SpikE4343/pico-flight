#ifndef __io_rc_h_INCLUDED__
#define __io_rc_h_INCLUDED__

#include "vector.h"
#include "stdbool.h"
#include "math_fixed.h"
#include "telemetry.h"

#define INPUT_CONTROL_ROLL 0
#define INPUT_CONTROL_PITCH 1
#define INPUT_CONTROL_YAW 2
#define INPUT_CONTROL_TROTTLE 3
#define NUM_AXIS_CONTROLS 4
#define INPUT_CONTROL_ARM 4

#define MAX_INPUT_CONTROL 32


void inputRxInit();
void inputRxUpdate();



#endif