add_library(core-system INTERFACE)

target_sources(core-system INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/system.c
)

target_include_directories(core-system INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(core-system INTERFACE)