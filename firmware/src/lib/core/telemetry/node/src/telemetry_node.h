#ifndef __telemetry_node_h_INCLUDED__
#define __telemetry_node_h_INCLUDED__

#include "telemetry_data.h"


typedef struct {
  int valueTableCount;
  int valueModBufferCount;
} TelemetryConfig_t;

void telemetry_node_init();
void telemetry_node_send(uint64_t start, uint64_t now);
void telemetry_node_recv(uint8_t byte);
void telemetry_node_update();

bool telemetry_node_register(TDataVar_t* dataVar);
bool telemetry_node_register_array(TDataVar_t* dataVar, int count);
bool telemetry_node_register_id();

void telemetry_node_sample(TDataVar_t* dataVar);
void telemetry_node_sample_at(TDataVar_t* dataVar, float now);

void telemetry_node_sample_array(TDataVar_t* dataVar, int count);

int telemetry_node_count();
TValue_t telemetry_node_get(uint32_t id);
TDataValueDesc_t *telemetry_node_get_desc(uint32_t id);


#endif