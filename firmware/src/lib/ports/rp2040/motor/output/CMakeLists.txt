add_library(motor-output-dshot-rp2040-pio INTERFACE)

pico_generate_pio_header(motor-output-dshot-rp2040-pio 
  ${CMAKE_CURRENT_LIST_DIR}/src/dshot_parallel.pio OUTPUT_DIR 
  ${CMAKE_CURRENT_LIST_DIR}/src)

target_sources(motor-output-dshot-rp2040-pio INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/motor_output_dshot_pio.c
)

target_include_directories(motor-output-dshot-rp2040-pio INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src 
)



target_link_libraries(motor-output-dshot-rp2040-pio INTERFACE
    core-math
    core-telemetry
    data-vars
    motor-common
    motor-output
    pico_stdlib 
    hardware_pio
    hardware_dma
    hardware_irq
)