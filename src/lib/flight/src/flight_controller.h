#ifndef flight_controller_INCLUDED
#define flight_controller_INCLUDED

#include "gyro_spi_mpu9250.h"
#include "motor_mixer.h"
#include "motor_output.h"
#include "telemetry.h"
//#include "display_controller.h"
#include "io_rc.h"
#include "motor_output_dshot.h"
#include "flight_attitude.h"
#include "osd.h"

#include "fc_vars.h"

// typedef struct
// {
//   double control;
//   double gyro;
//   double print;
// } FlightUpdateIntervalConfig_t;

// typedef struct
// {
//   float rc;
//   float super;
//   float expo;
// } ControlRates_t;

// typedef struct
// {
//   ControlRates_t rates[4];
// } ControlConfig_t;

void flightInit();
void flightGyroUpdateTask();
void flightInitVars();




#endif