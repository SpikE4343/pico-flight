add_library(camera INTERFACE)

target_sources(camera INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/camera.c      
)

target_include_directories(camera INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src )
    
target_link_libraries(camera INTERFACE 
  pico_stdlib 
  hardware_i2c
  core-telemetry-common
  data-vars
  )