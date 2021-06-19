add_library(motor-output INTERFACE)


target_sources(motor-output INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/motor_output.c
    ${CMAKE_CURRENT_LIST_DIR}/src/motor_output_dshot.c
)

target_include_directories(motor-output INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src 
)



target_link_libraries(motor-output INTERFACE
    core-math
    core-telemetry
    data-vars
    motor-common
)