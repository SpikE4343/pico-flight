add_library(data-vars INTERFACE)

target_sources(data-vars INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/data_vars.c
)

target_include_directories(data-vars INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/include  
)

target_link_libraries(data-vars INTERFACE
    core-telemetry
)