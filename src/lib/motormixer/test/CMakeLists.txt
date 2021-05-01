
project(motor_mixer_test CXX)

# add_library(motor_mixer_test INTERFACE)

target_sources(motor_mixer_test INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/motor_mixer_test.c
)

target_include_directories(motor_mixer_test INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src )
target_link_libraries(motor_mixer_test INTERFACE
    core-math
    motormixer
)
