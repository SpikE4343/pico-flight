cmake_minimum_required(VERSION 3.12)

add_library(web_server 
  http.c
  web_server.c
)

target_link_libraries(web_server
  usb_network_stack
  core-telemetry
  data-vars
  lwip
  tinyusb_device
)

target_compile_definitions(web_server PUBLIC
)



target_include_directories(web_server
 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
 )