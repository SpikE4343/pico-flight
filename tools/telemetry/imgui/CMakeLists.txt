

# find_package(OpenGL REQUIRED)
# include_directories(PUBLIC ${OPENGL_INCLUDE_DIR})

# find_package(GLEW REQUIRED)
# include_directories(${GLEW_INCLUDE_DIRS})


# include_directories(../../external/nuklear)



add_executable(pico-flight-gui
${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
        )

target_include_directories(pico-flight-gui PRIVATE 
  ${CMAKE_CURRENT_LIST_DIR}/src)

target_link_libraries(pico-flight-gui 
  # ${OPENGL_LIBRARIES}
  # ${GLEW_LIBRARIES}
  core-telemetry-common
  core-system-tools
  tools-shared-serial
  tools-shared-imgui
)



