#ifndef __telemetry_h_INCLUDED__
#define __telemetry_h_INCLUDED__

#include "telemetry_data.h"


typedef struct {
  int valueTableCount;
  int valueModBufferCount;
} TelemetryConfig_t;

void telemetryInit();
void telemetry_send(uint64_t start, uint64_t now);
void telemetry_recv(uint8_t byte);

bool telemetry_register(TDataVar_t* dataVar);
bool telemetry_register_array(TDataVar_t* dataVar, int count);
void telemetry_sample(TDataVar_t* dataVar);
void telemetry_sample_at(TDataVar_t* dataVar, float now);

void telemetry_sample_array(TDataVar_t* dataVar, int count);

TValue_t telemetry_get(uint32_t id);
TDataValueDesc_t *telemetry_get_desc(uint32_t id);


#endif