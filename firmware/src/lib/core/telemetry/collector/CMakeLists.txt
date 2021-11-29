add_library(core-telemetry-collector INTERFACE)

target_sources(core-telemetry-collector INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/telemetry-collector.c
)

target_include_directories(core-telemetry-collector INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(core-telemetry-collector INTERFACE
  core-system
  core-telemetry-common
  )