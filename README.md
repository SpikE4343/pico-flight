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
    - Still has some v-sync issues, probably from end of frame to start of next frame timing inconsistencies.
      - [ ] Convert dma to use a pacing timer for each line, so the timing is consitent
  - [ ] **PIO based camera stream**       
  - [ ] **Configuration Protocol**
    * Device maintains schema for transmition to configurator (any other device)
    * Key Value pairing treat like register map
    - [ ] Telemetry
    - [x] Add config data to telemetry
      - [x] Allow device to accept value mod messages to write to config data
      - [ ] Read/Write to "file" all config vars
      - [ ] Logging to flash/sdcard
    - [ ] Configurator
      - [ ] Native
        - [x] Openframeworks based via serial port with read/write support
      - [ ] PySide?
    - Web
      - [x] usb network endpoint
        - [x] basic http server 
        - [ ] route handling for subsystems
    - [ ] Host on device mass storage
      - [ ] Store current settings in human readable format.
      - [ ] Keep a backup of last settings?
    
