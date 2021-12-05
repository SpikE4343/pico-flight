#ifndef __telemetry_common_h_INCLUDED__
#define __telemetry_common_h_INCLUDED__

#include "telemetry_data.h"


typedef void (*telemetry_desc_callback_t)(TDataVarDesc_t* desc);

typedef struct {
  int valueTableCount;
  int valueModBufferCount;
  int stringBufferSize;
  bool autoRegisterVars;
} TelemetryConfig_t;

void telemetry_init();
void telemetry_update();

void telemetry_send();
void telemetry_recv(uint8_t byte);

void telemetry_set_desc_callback(telemetry_desc_callback_t callback);

bool telemetry_register_var(TDataVar_t* dataVar);
bool telemetry_register_var_array(TDataVar_t* dataVar, int count);
bool telemetry_register_id();

void telemetry_sample_var(TDataVar_t* dataVar);
void telemetry_sample_var_at(TDataVar_t* dataVar, float now);

void telemetry_sample_var_array(TDataVar_t* dataVar, int count);

bool telemetry_write_all_to_file(const char* filename, uint32_t modsFilter);

int telemetry_var_count();
TDataVar_t* telemetry_get_var(uint32_t id);
TDataVar_t* telemetry_get_var_by_name(const char* name);

uint32_t __time_critical_func(telemetry_write_data_frame)(TDataModPacket_t *packet);
uint32_t __time_critical_func(telemetry_write_desc_frame)(TDataDescFramePacket_t *packet,TDataVar_t *var);

DECL_EXTERN_DATA_VAR(tdv_telemetry_sample_buffer_count); 
DECL_EXTERN_DATA_VAR(tdv_telemetry_val_count);
DECL_EXTERN_DATA_VAR(tdv_telemetry_str_table);
DECL_EXTERN_DATA_VAR(tdv_telemetry_auto_register); 

#endif