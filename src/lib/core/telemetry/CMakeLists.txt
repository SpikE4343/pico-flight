add_library(core-telemetry INTERFACE)

target_sources(core-telemetry INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/telemetry.c
)

target_include_directories(core-telemetry INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(core-telemetry INTERFACE)