add_library(io_rc INTERFACE)

target_sources(io_rc INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/io_rc.c
        ${CMAKE_CURRENT_LIST_DIR}/src/io_rc_dma.c
        ${CMAKE_CURRENT_LIST_DIR}/src/io_rc_srxl2.c
)

target_include_directories(io_rc INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src  )

target_link_libraries(io_rc INTERFACE 
    core-math
    core-telemetry-common
    data-vars
    srxl2
    hardware_uart
    hardware_dma
)