add_library(srxl2 INTERFACE)

target_sources(srxl2 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/spm_srxl.c
)

target_include_directories(srxl2 INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR} 
)

# target_link_libraries(srxl2 INTERFACE 
#     pico_stdlib 
#     hardware_pio
#     hardware_dma
# )