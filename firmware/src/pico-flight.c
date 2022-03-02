

#define USING_PICO_PROBE 1 //PICO_ON_DEVICE

#if USING_PICO_PROBE
#include "pico/stdio_uart.h"
#include "pico/stdlib.h"
#else
#include "pico/stdlib.h"
#endif

#include <stdio.h>
#include "flight_controller.h"
#include "data_vars.h"

int main()
{
   

#if USING_PICO_PROBE  
  stdio_uart_init_full(uart0, 400000, 16, 17 );
#else
  stdio_init_all();
#endif

  telemetry_init();
  data_vars_init();

  flightInit();

  return 0;
}
