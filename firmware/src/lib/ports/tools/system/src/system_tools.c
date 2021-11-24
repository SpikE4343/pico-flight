
#include "system.h"
#include "system_native.h"
#include "time.h"


uint64_t system_native_get_time_us(void)
{
  // TODO: system time?
  return time(NULL);
}

uint32_t system_native_clock_hz(void)
{
 return 12000000;
}

