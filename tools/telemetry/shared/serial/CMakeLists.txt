add_library(tools-shared-serial INTERFACE)

target_sources(tools-shared-serial INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/serial_linux.c
)

target_include_directories(tools-shared-serial INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src)
    
target_link_libraries(tools-shared-serial INTERFACE
  # core-system
  )

