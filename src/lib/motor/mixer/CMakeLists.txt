add_library(motor-mixer INTERFACE)

target_sources(motor-mixer INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/motor_mixer.c
)

target_include_directories(motor-mixer INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src )
target_link_libraries(motor-mixer INTERFACE
    core-math
    motor-common
    data-vars
)

# add_subdirectory(test)