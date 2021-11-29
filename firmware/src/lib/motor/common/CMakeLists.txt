add_library(motor-common INTERFACE)

target_sources(motor-common INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/motor_common.c
)

target_include_directories(motor-common INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src )
    
target_link_libraries(motor-common INTERFACE
    core-math
    core-telemetry-common
    data-vars
)
