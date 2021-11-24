#ifndef flight_pid_INCLUDED
#define flight_pid_INCLUDED

#include "math_util.h"
#include "filters.h"
#include "vector.h"
#include "io_rc.h"
#include "gyro_spi_mpu9250.h"

#define PID_ROLL 0
#define PID_PITCH 1
#define PID_YAW 2
#define PID_AXIS_COUNT 3

#define PID_P 0
#define PID_I 1
#define PID_D 2
#define PID_F 3


void flightAttitudeInit();
void flightAttitudeUpdate(TDataVar_t* outputs, TDataVar_t* inputs, TDataVar_t* gyro, float dT);



#endif