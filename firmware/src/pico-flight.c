#include <stdio.h>
#include "pico/stdlib.h"
#include "flight_controller.h"
#include "data_vars.h"

#define USING_PICO_PROBE 1

int main()
{
  stdio_init_all();

#if USING_PICO_PROBE
  gpio_set_function(16, GPIO_FUNC_UART);
  gpio_set_function(17, GPIO_FUNC_UART);
  uart_set_fifo_enabled(uart0, true);
#endif

  telemetry_init();
  data_vars_init();

  flightInit();

  return 0;
}
