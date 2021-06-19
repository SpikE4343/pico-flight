add_library(core-system-rp2040 INTERFACE)

target_sources(core-system-rp2040 INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/system_rp2040.c
)

target_include_directories(core-system-rp2040 INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src)
    
target_link_libraries(core-system-rp2040 INTERFACE
  core-system
  pico_stdlib
  )
