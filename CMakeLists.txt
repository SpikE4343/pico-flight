cmake_minimum_required(VERSION 3.12)


set(PF_BUILD_TOOLS OFF CACHE BOOL "Build tools instead of firmware")

if( NOT PF_BUILD_TOOLS)
  # Pull in PICO SDK (must be before project)
  include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


project(pico-flight C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

enable_testing()
#set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

if( NOT PF_BUILD_TOOLS)
# Initialize the SDK
  pico_sdk_init()
endif()

add_subdirectory(firmware)

if( PF_BUILD_TOOLS)
  add_subdirectory(tools)
endif()

