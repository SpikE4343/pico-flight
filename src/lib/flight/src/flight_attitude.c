

#include "fc_vars.h"
#include "flight_attitude.h"
#include <string.h>


// ---------------------------------------------------------------
void flightAttitudeInit()
{
  
}

TDataVar_t* PIDF_Gains[] = {
  &tdv_fc_pidf_k_roll[0],
  &tdv_fc_pidf_k_pitch[0],
  &tdv_fc_pidf_k_yaw[0]
};


TDataVar_t* PIDF_Values[] = {
  &tdv_fc_pidf_v_roll[0],
  &tdv_fc_pidf_v_pitch[0],
  &tdv_fc_pidf_v_yaw[0]
};

// ---------------------------------------------------------------
void flightAttitudePID(TDataVar_t* outputs, TDataVar_t* inputs, TDataVar_t* gyro, float dT)
{
  for (int a = 0; a < 3; ++a)
  {
    tdv_fc_pid_sp_delta[a].v.f32 = inputs[a].v.f32 - tdv_fc_pid_sp[a].v.f32;
    tdv_fc_pid_sp_error[a].v.f32 = inputs[a].v.f32 - gyro[a].v.f32;
    tdv_fc_pid_pv_error[a].v.f32 = tdv_fc_pid_pv[a].v.f32 - gyro[a].v.f32;

    tdv_fc_pid_pv[a].v.f32 = gyro[a].v.f32;

    // TODO: clamp values to reasonable limits

    // P-Term
    PIDF_Values[a][PID_P].v.f32 = PIDF_Gains[a][PID_P].v.f32 * tdv_fc_pid_sp_error[a].v.f32;

    // I-Term
    PIDF_Values[a][PID_I].v.f32 += PIDF_Gains[a][PID_I].v.f32 * tdv_fc_pid_sp_error[a].v.f32;

    // D-Term
    PIDF_Values[a][PID_D].v.f32 = PIDF_Gains[a][PID_D].v.f32 * tdv_fc_pid_pv_error[a].v.f32 * dT;

    // F-Term
    PIDF_Values[a][PID_F].v.f32 = PIDF_Gains[a][PID_F].v.f32 * tdv_fc_pid_sp_delta[a].v.f32 * dT;

    // PID Sum
    outputs[a].v.f32 = tdv_fc_pid_sum[a].v.f32 = 
      PIDF_Values[a][PID_P].v.f32 + 
      PIDF_Values[a][PID_I].v.f32 + 
      PIDF_Values[a][PID_D].v.f32 + 
      PIDF_Values[a][PID_F].v.f32;

    // printf("[%u] = % .3f, % .3f, % .3f, % .3f, % .3f\n", a,
    //   s.PID[a][PID_P],
    //   s.PID[a][PID_I],
    //   s.PID[a][PID_D],
    //   s.PID[a][PID_F],
    //   dT
    //   );
  }

  // pass throttle through for now....
  outputs[3].v.f32 = inputs[3].v.f32;
}

// ---------------------------------------------------------------
void flightAttitudeUpdate(TDataVar_t* outputs, TDataVar_t* inputs, TDataVar_t* gyro, float dT)
{
  flightAttitudePID(outputs, inputs, gyro, dT);
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


// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pidf_k_roll )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.P.g", "Roll Proportional response gain", f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.I.g", "Roll Integral response gain", f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.D.g", "Roll Derivative response gain", f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(1.0f, "fc.at.roll.F.g", "Roll Feed-Forward gain", f32, Tdm_RW | Tdm_config),
END_DEF_DV_ARRAY();

// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pidf_k_pitch )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.P.g", "Pitch Proportional response gain", f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.I.g", "Pitch Integral response gain", f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.D.g", "Pitch Derivative response gain", f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(1.0f, "fc.at.pitch.F.g", "Pitch Feed-Forward gain", f32, Tdm_RW | Tdm_config),
END_DEF_DV_ARRAY();

// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pidf_k_yaw )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.P.g", "Yaw Proportional response gain", f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.I.g", "Yaw Integral response gain", f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.D.g", "Yaw Derivative response gain", f32, Tdm_RW | Tdm_config),
  DEF_DV_ARRAY_ITEM_NAMED(1.0f, "fc.at.yaw.F.g", "Yaw Feed-Forward gain", f32, Tdm_RW | Tdm_config),
END_DEF_DV_ARRAY();


// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pidf_v_roll )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.P.v", "Roll Proportional response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.I.v", "Roll Integral response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.D.v", "Roll Derivative response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.F.v", "Roll Feed-Forward value", f32, Tdm_read| Tdm_realtime),
END_DEF_DV_ARRAY();

// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pidf_v_pitch )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.P.v", "Pitch Proportional response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.I.v", "Pitch Integral response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.D.v", "Pitch Derivative response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.F.v", "Pitch Feed-Forward value", f32, Tdm_read| Tdm_realtime),
END_DEF_DV_ARRAY();

// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pidf_v_yaw )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.P.v", "Yaw Proportional response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.I.v", "Yaw Integral response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.D.v", "Yaw Derivative response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.F.v", "Yaw Feed-Forward value", f32, Tdm_read| Tdm_realtime),
END_DEF_DV_ARRAY();



// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pid_sum )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.sum", "Roll response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.sum", "Pitch response value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.sum", "Yaw response value", f32, Tdm_read| Tdm_realtime),
END_DEF_DV_ARRAY();

// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pid_sp )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.sp.v", "Roll setpoint value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.sp.v", "Pitch setpoint value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.sp.v", "Yaw setpoint value", f32, Tdm_read| Tdm_realtime),
END_DEF_DV_ARRAY();

// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pid_sp_error )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.sp.error", "Roll setpoint gyro error value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.sp.error", "Pitch setpoint gyro error value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.sp.error", "Yaw setpoint gyro error value", f32, Tdm_read| Tdm_realtime),
END_DEF_DV_ARRAY();

// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pid_sp_delta )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.sp.delta", "Roll setpoint delta value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.sp.delta", "Pitch setpoint delta value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.sp.delta", "Yaw setpoint delta value", f32, Tdm_read| Tdm_realtime),
END_DEF_DV_ARRAY();

// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pid_pv_error )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.pv.error", "Roll gyro error value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.pv.error", "Pitch gyro error value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.pv.error", "Yaw gyro error value", f32, Tdm_read| Tdm_realtime),
END_DEF_DV_ARRAY();

// ---------------------------------------------------------------
BEGIN_DEF_DV_ARRAY( tdv_fc_pid_pv )
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.roll.pv.v", "Roll gyro value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.pitch.pv.v", "Pitch gyro value", f32, Tdm_read| Tdm_realtime),
  DEF_DV_ARRAY_ITEM_NAMED(0.0f, "fc.at.yaw.pv.v", "Yaw gyro value", f32, Tdm_read| Tdm_realtime),
END_DEF_DV_ARRAY();
