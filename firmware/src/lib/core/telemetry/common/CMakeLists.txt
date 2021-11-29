add_library( core-telemetry-common INTERFACE)

target_sources( core-telemetry-common INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/telemetry-common.c
  )

target_include_directories( core-telemetry-common INTERFACE 
  ${CMAKE_CURRENT_LIST_DIR}/src
  )

target_link_libraries( core-telemetry-common INTERFACE
  core-system
  )