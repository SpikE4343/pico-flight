#include "system.h"
#include "system_native.h"

// ---------------------------------------------------------------
uint64_t system_time_us(void)
{
  return system_native_get_time_us();
}

// ---------------------------------------------------------------
uint32_t system_clock_hz(void)
{
  return system_native_clock_hz();
}