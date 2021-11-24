# Telemetry Core Library

## Telmetry variables
  * Basically global variables that can be remotely modified
  * Supports sampling and streaming current state
  * Max of 32 bit numeric data 
### Telemetry Spec


 * Array of Key value pairs
 * Global 32-bit Field Id
  * little endian for easy struct loading
 * Variable value size
  * Max of 3 bytes
 * ```| [31:0] Id | [7:0] Data Type | [7:0] Len | Data [0...Len] |```