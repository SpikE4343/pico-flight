cmake_minimum_required(VERSION 3.12)

add_library(usb_network_stack 
  usb_descriptors.c
  dhserver.c
  dnserver.c
  rndis_reports.c)

target_link_libraries(usb_network_stack
    pico_stdlib
    pico_stdio
    tinyusb_device
    lwip
)

target_include_directories(usb_network_stack PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})