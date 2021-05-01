add_library(motormixer INTERFACE)

target_sources(motormixer INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/motor_mixer.c
)

target_include_directories(motormixer INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src )
target_link_libraries(motormixer INTERFACE
    core-math
)

# add_subdirectory(test)