add_library(core-math INTERFACE)

target_sources(core-math INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/filters.c
        ${CMAKE_CURRENT_LIST_DIR}/src/math_game.c
        ${CMAKE_CURRENT_LIST_DIR}/src/vector.c
)

target_include_directories(core-math INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(core-math INTERFACE)