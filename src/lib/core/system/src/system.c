#include "system.h"
#include "system_native.h"

// ---------------------------------------------------------------
uint64_t system_time_us(void)
{
  return system_native_get_time_us();
}