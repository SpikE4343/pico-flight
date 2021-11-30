add_library(tools-shared-imgui INTERFACE)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)


set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/../../external/imgui)

message("SDL2_INCLUDE_DIRS = '${SDL2_INCLUDE_DIRS}'")

target_sources(tools-shared-imgui INTERFACE
  ${IMGUI_DIR}/imgui.cpp 
  ${IMGUI_DIR}/imgui_demo.cpp 
  ${IMGUI_DIR}/imgui_draw.cpp 
  ${IMGUI_DIR}/imgui_tables.cpp 
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp 
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp

)

target_include_directories(tools-shared-imgui INTERFACE 
  ${SDL2_INCLUDE_DIRS}
  ${CMAKE_CURRENT_LIST_DIR}/src  
  ${IMGUI_DIR}/
  ${IMGUI_DIR}/backends
  )
    
target_link_libraries(tools-shared-imgui INTERFACE
  ${SDL2_LIBRARIES}
  OpenGL::GL 
  ${CMAKE_DL_LIBS}
  # core-system
  )

