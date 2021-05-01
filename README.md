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
       
  - [ ] **Configuration Protocol**
    * Device maintains schema for transmition to configurator (any other device)
    * Key Value pairing treat like register map
  - [ ] Telemetry
   - [ ] Add config data to telemetry
    - [ ] Allow device to accept value mod messages to write to config data
   - [ ] Logging to flash/sdcard
