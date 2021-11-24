if(NOT PF_BUILD_TOOLS)
  add_executable(pico-flight
          pico-flight.c
          )

  target_include_directories(pico-flight PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR})

  # Pull in our pico_stdlib which aggregates commonly used features
  target_link_libraries(pico-flight 
      pico_stdlib
      flight
      core-system-rp2040
      core-telemetry-rp2040
      motor-output-dshot-rp2040-pio
    )

  # enable usb output, disable uart output
# pico_enable_stdio_usb(pico-flight 1)
pico_enable_stdio_uart(pico-flight 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(pico-flight)

# add url via pico_set_program_url
#example_auto_set_url(pico-flight)
endif()

add_subdirectory(lib)

