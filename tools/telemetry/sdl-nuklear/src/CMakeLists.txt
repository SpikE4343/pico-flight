add_executable(telepi
        telemetry.c
        )

target_include_directories(telepi PRIVATE 
  ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(telepi 
  ${SDL2_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  core-telemetry
  core-system-tools
  core-telemetry-tools
)

