#ifndef __tools_telemetry_data_var_ui_h_INCLUDED__
#define __tools_telemetry_data_var_ui_h_INCLUDED__

#include "lib_imgui.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "telemetry.h"
#include "telemetry_native.h"

#ifdef __cplusplus
}
#endif

bool InputUInt32(const char *label, uint32_t *v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);

bool ShowDataVarEditor(TDataVar_t *v);

#endif