add_library( core-telemetry-common INTERFACE)

if(PF_BUILD_TOOLS)
  target_compile_options(core-telemetry-common INTERFACE -DTELEMETRY_PRE_ALLOC=1 )
endif()

target_sources( core-telemetry-common INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/telemetry-common.c
  )

target_include_directories( core-telemetry-common INTERFACE 
  ${CMAKE_CURRENT_LIST_DIR}/src
  )

target_link_libraries( core-telemetry-common INTERFACE
  core-system
  )