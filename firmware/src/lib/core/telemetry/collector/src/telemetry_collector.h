#ifndef __telemetry_collector_h_INCLUDED__
#define __telemetry_collector_h_INCLUDED__

#include "telemetry_data.h"


typedef void (*telemetry_collector_desc_callback_t)(TDataValueDesc_t* desc);

typedef struct {
  int valueTableCount;
  int valueModBufferCount;
  bool autoRegisterVars;
} TelemetryConfig_t;

void telemetry_collector_init();
void telemetry_collector_update();

void telemetry_collector_send();
void telemetry_collector_recv(uint8_t byte);

void telemetry_collector_set_desc_callback(telemetry_collector_desc_callback_t callback);


int telemetry_count();
TDataVar_t telemetry_get(uint32_t id);

#endif