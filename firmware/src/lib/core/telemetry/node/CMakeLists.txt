
add_library(core-telemetry-node INTERFACE)

target_sources(core-telemetry-node INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/telemetry-node.c
  )

target_include_directories(core-telemetry-node INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(core-telemetry-node INTERFACE
  core-system
  core-telemetry-common
  )