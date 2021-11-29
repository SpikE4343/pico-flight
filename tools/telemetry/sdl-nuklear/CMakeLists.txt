
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(PUBLIC ${OPENGL_INCLUDE_DIR})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})


# include_directories(../../external/nuklear)


add_executable(telepi
${CMAKE_CURRENT_LIST_DIR}/src/main.c
        )

target_include_directories(telepi PRIVATE 
  ${CMAKE_CURRENT_LIST_DIR}/src)

target_link_libraries(telepi 
  ${SDL2_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  core-telemetry-common
  # core-telemetry-collector
  # core-telemetry-node
  core-system-tools
  tools-shared-serial
  # core-telemetry-tools
)



