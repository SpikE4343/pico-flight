# pico-flight

## I/O Radio Control Input Library

* Provide parsing of radio control input data for flight controls, telemetry, and link status

## Protocols
  
  - [ ] **SRXL2**
    - [x] Basic parsing of control data via spectrum public library: []()
    - [ ] Try UART interrupt method to fill rx buffer
    - [ ] Add dma support to since current polling implemention can't keep up with 400k buad with control frame size of 38 bytes

  - [ ] **SBUS**
    - [ ] TODO

  - [ ] **Ghost** TODO

## Features
  - [ ] **Telemetry**