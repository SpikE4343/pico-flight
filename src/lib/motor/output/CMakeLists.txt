add_library(motor-output INTERFACE)

pico_generate_pio_header(motor-output ${CMAKE_CURRENT_LIST_DIR}/src/dshot_parallel.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

target_sources(motor-output INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/motor_output_dshot.c
    ${CMAKE_CURRENT_LIST_DIR}/src/motor_output_dshot_pio.c
)

target_include_directories(motor-output INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src 
)



target_link_libraries(motor-output INTERFACE
    core-math
    motor-common
    pico_stdlib 
    hardware_pio
    hardware_dma
    hardware_irq
)