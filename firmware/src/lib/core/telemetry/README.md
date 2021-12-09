# Telemetry Core Library

## Telmetry variables
  * Basically global variables that can be remotely modified
  * Supports sampling and streaming current state
  * Max of 32 bit numeric data 
 ### TODO:
  - [ ] Save/Load
    - [ ] file save
      - [x] text based
      - [ ] binary append of value mods for logging telemetry data
        - [ ] can be used in tool to log sessions
    - [ ] file load
      - [ ] text based
  - [ ] Split common code into 
    - [ ] node (sender)
    - [ ] collector (recv/logging) 
  - [ ] string based id mode?
  - [ ] Centralized sampling framework
    - [ ] Allow setting sampling settings remotely
      - [ ] Sample Rate
      - [ ] Sample Enable/Disable
  - [ ] Create `Command` mod_type flags
    - [ ] Var when set will trigger some processing
  - [ ] DataVars define more meta data
    - [ ] min
    - [ ] max
    - [ ] step
    - [ ] Enum values
      - [ ] integer->string mapping?
      - [ ] need the reverse mapping for text based input
    - [ ] Configurator layout?
  
