add_library(core-telemetry-rp2040 INTERFACE)

target_sources(core-telemetry-rp2040 INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/telemetry_native_rp2040.c
)

target_include_directories(core-telemetry-rp2040 INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src)

target_link_libraries(core-telemetry-rp2040 INTERFACE
  core-telemetry)