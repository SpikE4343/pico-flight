#ifndef __motor_output_h_INCLUDED__
#define __motor_output_h_INCLUDED__

#include <stdint.h>
#include "motor_common.h"





// inputs from mixer


// output values to esc/motors
typedef struct
{
  uint8_t motorCount;
  uint16_t motor[MAX_MOTORS];
} MotorOutputValues_t;

void motorOutputInit();
void motorOutputSet(TDataVar_t *output);

DECL_EXTERN_DV_ARRAY( tdv_motor_output) ;
DECL_EXTERN_DV_ARRAY( tdv_motor_output_pin );
DECL_EXTERN_DATA_VAR( tdv_motor_startup_delay_ms );

DECL_EXTERN_DATA_VAR( tdv_motor_output_min );
DECL_EXTERN_DATA_VAR( tdv_motor_output_idle );
DECL_EXTERN_DATA_VAR( tdv_motor_output_max );

DECL_EXTERN_DATA_VAR( tdv_motor_output_rate );

#endif