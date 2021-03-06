
#include "system.h"
#include "system_native.h"

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

uint64_t system_native_get_time_us(void)
{
  // Reading low latches the high value
  uint32_t lo = timer_hw->timelr;
  uint32_t hi = timer_hw->timehr;
  return ((uint64_t)hi << 32u) | lo;
}

uint32_t system_native_clock_hz(void)
{
 return clock_get_hz(clk_sys);
}

