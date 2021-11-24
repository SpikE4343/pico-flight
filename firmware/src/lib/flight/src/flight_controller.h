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


void flightInit();
void flightGyroUpdateTask();
void flightInitVars();




#endif