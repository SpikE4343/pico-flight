# pico-flight


### Telemetry Spec


 * Array of Key value pairs
 * Global 32-bit Field Id
  * little endian for easy struct loading
 * Variable value size
 * ```| [31:0] Id | [7:0] Data Type | [7:0] Len | Data [0...Len] |```