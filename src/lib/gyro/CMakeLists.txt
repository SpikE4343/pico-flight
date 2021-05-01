add_library(gyro INTERFACE)

target_sources(gyro INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/gyro_spi_mpu9250.c
        ${CMAKE_CURRENT_LIST_DIR}/src/gyro_vars.c
)

target_include_directories(gyro INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src )
target_link_libraries(gyro INTERFACE 
  pico_stdlib 
  hardware_spi
  core-telemetry
  )