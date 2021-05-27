# pico-flight




## Plans
  - [ ] **PIO based on screen display**
    - [ ] Vertical and horizontal sync tracking of existing NTSC/PAL signal
    - [x] First steps to attempt to get simple "tvout" signal working, try starting from vga examples
    - [x] Minimum voltages required
       | Status | Voltage | Notes
       |--- | ---| --- |
       |  | NC | Not connected, leave existing signal intact|
       |  | 0V | sync level |
       |  | 1V | Black/Blanking  |
       |  | 3.3v | White |
    - [ ] Still has some v-sync issues, probably from end of frame to start of next frame timing inconsistencies.
      - [ ] Double buffer dma output so remove interrupt delay when starting the next frame
       
  - [ ] **Configuration Protocol**
    * Device maintains schema for transmition to configurator (any other device)
    * Key Value pairing treat like register map
    - [ ] Telemetry
    - [x] Add config data to telemetry
      - [x] Allow device to accept value mod messages to write to config data
      - [ ] Read/Write to "file" all config vars
      - [ ] Logging to flash/sdcard
    - [ ] Configurator
      - [ ] Native?
        -[ ] PySide?
      - [ ] Web?
        -[ ] usb network endpoint to host rest like api?
      - [ ] Host on device mass storage?
    
