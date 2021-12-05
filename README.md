# pico-flight

# How To Build
 
 * Current two ways to build
  * For device RPi pico board
  * Tools
    * `PF_BUILD_TOOLS=ON`
 - [ ] Investigate using "target or platform" instead of custom variable
### Firmare for RP2040 (Pico)

```
mkdir builds
mkdir builds/firmware
cd builds/firmware
cmake ../..
make
```

### Tools
```
mkdir builds
mkdir builds/tools
cd builds/tools
cmake ../.. -D PF_BUILD_TOOLS=ON
make
```


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
    - [x] Still has some v-sync issues, probably from end of frame to start of next frame timing inconsistencies.
      - [x] Convert to chained dma transfer to automatically restart the frame.
        > Pacing timers only govern when an individual transfer happens, not the entire channel trigger
  - [ ] **PIO based camera stream**       
  - [ ] **Configuration Protocol**
    * Device maintains schema for transmition to configurator (any other device)
    * Key Value pairing treat like register map
    - [ ] Telemetry
    - [x] Add config data to telemetry
      - [x] Allow device to accept value mod messages to write to config data
      - [ ] Add littlefs for flash file storage
        - [ ] Read/Write to "file" all config vars
          - [x] Write of text based config vars 
        - [ ] Logging to flash/sdcard
        
        
    - [ ] Configurator
      - [ ] Native
        - [x] Openframeworks based via serial port with read/write support
          * OF dependency is pretty large and I lost the ofApp.cpp file :(
        - [x] Reimplementing using https://github.com/Immediate-Mode-UI/Nuklear and SDL/OpenGL
        - [x] Reimplemented using Dear Imgui
          - [ ] Debug why settings not applying on device when sent from tool
    - Web
      - [x] usb network endpoint
        - [x] basic http server 
        - [ ] route handling for subsystems
    - [ ] Host on device flash
      - [ ] Store current settings in human readable format.
      - [ ] Keep a backup of last settings?
    
