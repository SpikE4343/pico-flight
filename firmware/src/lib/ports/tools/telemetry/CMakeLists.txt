add_library(core-telemetry-tools INTERFACE)

target_sources(core-telemetry-tools INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/telemetry_native_tools.c
)

target_include_directories(core-telemetry-tools INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src)

target_link_libraries(core-telemetry-tools INTERFACE
  core-telemetry)