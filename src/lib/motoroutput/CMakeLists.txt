add_library(motoroutput INTERFACE)

pico_generate_pio_header(motoroutput ${CMAKE_CURRENT_LIST_DIR}/src/dshot_parallel.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

target_sources(motoroutput INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/motor_output_dshot.c
    ${CMAKE_CURRENT_LIST_DIR}/src/motor_output_dshot_pio.c
)

target_include_directories(motoroutput INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src 
)



target_link_libraries(motoroutput INTERFACE
    core-math
    pico_stdlib 
    hardware_pio
    hardware_dma
    hardware_irq
)