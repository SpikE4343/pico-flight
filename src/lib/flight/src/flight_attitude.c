
#include "flight_attitude.h"
#include <string.h>

typedef struct
{
  float PID[3][4];
  float PIDSum[3];
  float lastPV[3];
  float lastSP[3];
  float spError[3];
  float spDelta[3];
  float pvError[3];
} FlightAttitudeState_t;

FlightPIDControllerConfig_t *c;
FlightAttitudeState_t s;

void flightAttitudePIDInit(FlightPIDControllerConfig_t *info)
{
  c = info;
  memset(&s, 0, sizeof(s));
}

Vector4f_t flightAttitudePID(Vector4f_t inputs, Vector3f_t gyro, float dT)
{
  Vector4f_t output = inputs;

  for (int a = 0; a < 3; ++a)
  {
    s.spDelta[a] = inputs.axis[a] - s.lastSP[a];
    s.spError[a] = inputs.axis[a] - gyro.axis[a];
    s.pvError[a] = s.lastPV[a] - gyro.axis[a];

    s.lastPV[a] = gyro.axis[a];

    // TODO: clamp values to reasonable limits

    // P-Term
    s.PID[a][PID_P] = c->PID[a][PID_P] * s.spError[a];

    // I-Term
    s.PID[a][PID_I] += c->PID[a][PID_I] * s.spError[a];

    // D-Term
    s.PID[a][PID_D] = c->PID[a][PID_D] * s.pvError[a] * dT;

    // F-Term
    s.PID[a][PID_F] = c->PID[a][PID_F] * s.spDelta[a] * dT;

    // PID Sum
    output.axis[a] = s.PIDSum[a] = s.PID[a][PID_P] + s.PID[a][PID_I] + s.PID[a][PID_D] + s.PID[a][PID_F];

    // printf("[%u] = % .3f, % .3f, % .3f, % .3f, % .3f\n", a,
    //   s.PID[a][PID_P],
    //   s.PID[a][PID_I],
    //   s.PID[a][PID_D],
    //   s.PID[a][PID_F],
    //   dT
    //   );
  }

  return output;
}

Vector4f_t flightAttitudeUpdate(Vector4f_t inputs, Vector3f_t gyro, float dT)
{
  return flightAttitudePID(inputs, gyro, dT);
}

// FlightPIDControllerConfig_t
// //
// data_type_write_decl(FlightPIDControllerConfig_t)
// {
//   data_set_static_array_of(Vector4f_t, PID, 3);
// }

// data_type_read_decl(FlightPIDControllerConfig_t)
// {
//   data_get_static_array_of(Vector4f_t, PID, 3);
// }