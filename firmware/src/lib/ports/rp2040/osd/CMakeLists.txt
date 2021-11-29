add_library(rp2040_osd INTERFACE)

pico_generate_pio_header(rp2040_osd ${CMAKE_CURRENT_LIST_DIR}/src/osd.pio 
  OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

target_sources(rp2040_osd INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/osd.c
)

target_include_directories(rp2040_osd INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src 
)



target_link_libraries(rp2040_osd INTERFACE
    pico_stdlib 
    hardware_pio
    hardware_dma
    hardware_irq
    core-telemetry-common
    data-vars
)