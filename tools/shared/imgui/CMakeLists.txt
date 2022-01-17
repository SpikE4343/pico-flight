add_library(tools-shared-imgui INTERFACE)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

find_package(OpenGL REQUIRED)

# Manually compile gui dependencies into one library


set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/../../external/imgui)
set(IMPLOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../external/implot)

message("SDL2_INCLUDE_DIR = '${SDL2_INCLUDE_DIR}'")

target_sources(tools-shared-imgui INTERFACE
  ${IMGUI_DIR}/imgui.cpp 
  # TODO: exclude demo from release builds
  ${IMGUI_DIR}/imgui_demo.cpp 
  ${IMGUI_DIR}/imgui_draw.cpp 
  ${IMGUI_DIR}/imgui_tables.cpp 
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp 
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp

  ${IMPLOT_DIR}/implot.cpp
  ${IMPLOT_DIR}/implot_items.cpp

  # TODO: exclude demo from release builds
  ${IMPLOT_DIR}/implot_demo.cpp

)

target_include_directories(tools-shared-imgui INTERFACE 
  ${SDL2_INCLUDE_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/src  
  ${IMGUI_DIR}/
  ${IMGUI_DIR}/backends
  ${IMPLOT_DIR}/
  )
    
target_link_libraries(tools-shared-imgui INTERFACE
  ${SDL2_LIBRARY}
  OpenGL::GL 
  ${CMAKE_DL_LIBS}
  # core-system
  )

