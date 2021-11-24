add_library(core-system-tools INTERFACE)

target_sources(core-system-tools INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/system_tools.c
)

target_include_directories(core-system-tools INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src)
    
target_link_libraries(core-system-tools INTERFACE
  core-system
  )
