#ifndef __telemetry_native_h_INCLUDED__
#define __telemetry_native_h_INCLUDED__

#include <stdint.h>

typedef void (*telemetry_native_send_callback_t)(int sentCount);

void telemetry_native_init(telemetry_native_send_callback_t sendCB);
void telemetry_native_deinit();
void telemetry_native_send(int length, uint8_t* packets, int packetCount);
void telemetry_native_recv(int max);
bool telemetry_native_sending();

// TODO: add malloc functions


#endif