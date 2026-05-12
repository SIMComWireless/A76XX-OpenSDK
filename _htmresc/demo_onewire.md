# demo_onewire

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_onewire.c

## Overview

This demo demonstrates the OneWire bus interface on SIMCom modules, specifically targeting the CT1820B/DS18B20-family temperature sensor. It creates a dedicated task that initializes the OneWire bus and periodically communicates with the sensor. The demo includes functions for sending a capture command and reading the temperature value.

## Main Features

- OneWire bus initialization via `sAPI_OneWireInit()`
- OneWire reset and device presence detection
- OneWire byte write for sending commands to slave devices
- OneWire byte read for reading data from slave devices
- CT1820B temperature sensor protocol implementation (skip ROM + convert T + read scratchpad)
- Dedicated RTOS task for OneWire processing with periodic polling
- Temperature reading and debug output

## Key Header Dependencies

- `simcom_onewire.h` -- Provides `sAPI_OneWireInit()`, `sAPI_OneWireSetupReset()`, `sAPI_OneWireWriteByte()`, and `sAPI_OneWireRead()` prototypes
- `simcom_os.h` -- Provides RTOS primitives: `sAPI_TaskCreate()`, `sAPI_TaskSleep()`, `sTaskRef`, `SC_STATUS`, `SC_SUCCESS`
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug output

## SDK APIs Observed

- `sAPI_OneWireInit()` -- Initialize the OneWire bus hardware interface
- `sAPI_OneWireSetupReset()` -- Perform the OneWire reset pulse and check for device presence; returns >0 if a device responded
- `sAPI_OneWireWriteByte(byte)` -- Send a single byte over the OneWire bus; returns 0 on success
- `sAPI_OneWireRead(buffer, len)` -- Read multiple bytes from the OneWire bus into a buffer; returns 0 on success
- `sAPI_TaskCreate(&taskRef, stack, stackSize, priority, name, entryFunc, arg)` -- Create an RTOS task
- `sAPI_TaskSleep(ticks)` -- Sleep the current task for the specified number of ticks (5ms per tick)

## Usage

1. Call `OneWireDemo()` from the application initialization code.
2. `OneWireDemo()` creates a task named "oneWireProcesser" with 3KB stack, priority 150.
3. The task entry function `sTask_OneWireProcesser()` calls `sAPI_OneWireInit()` to set up the bus.
4. The task enters an infinite loop, sleeping 200 ticks (1 second) between iterations.
5. When `ONE_WIRE_DEMO_ENABLE` is defined, the loop can call `ct1820b_get_temperature()` instead of the raw write.
6. The `ct1820b_send_capture_cmd()` function performs: reset, check presence, send Skip ROM (0xCC) + Convert T (0x44), then wait 50ms for conversion.
7. The `ct1820b_get_temperature()` function calls capture, then reads 9 bytes of scratchpad data; byte 0 is the temperature LSB.

## Implementation Notes

- The entire OneWire functional code is wrapped in `#ifdef ONE_WIRE_DEMO_ENABLE` (currently disabled). The task is always created but the loop only does `sAPI_OneWireWriteByte(0xab)` when the define is off.
- The CT1820B protocol uses:
  - `0xCC` (Skip ROM) -- address all devices on the bus
  - `0x44` (Convert T) -- initiate temperature conversion
  - `0xAB` (unused in standard DS18B20 -- may be a custom command or placeholder)
  - Read scratchpad (9 bytes) to get the temperature value
- `sAPI_OneWireSetupReset()` returns a value >= 1 when a device is present; values < 1 indicate no device detected.
- Temperature value is stored in `read_data[0]` (the LSB of the scratchpad). For full 12-bit resolution, the MSB would be in `read_data[1]`.
- The task stack is statically allocated: `static UINT8 oneWireProcesserStack[1024*3]` (3072 bytes).
- `sAPI_TaskSleep(200)` equals approximately 1 second (assuming 5ms tick period).
- The task reference `oneWireProcesser` is checked for NULL before creation to prevent creating duplicate tasks.

## Best Practices

- Call `sAPI_OneWireInit()` only once, before any OneWire operations.
- Always check the return value of `sAPI_OneWireSetupReset()` before sending commands -- if no device is present, subsequent operations will fail.
- Use Skip ROM (0xCC) when only one device is on the bus; use Match ROM (0x55) with the device's 64-bit ROM code when multiple devices share the bus.
- Allow sufficient delay after Convert T (0x44) for the temperature conversion to complete (at least 750ms for 12-bit resolution, 50ms is marginal for lower resolutions).
- Read all 9 scratchpad bytes and verify the CRC (byte 8) for reliable temperature data.
- Allocate adequate stack size for the OneWire task (3KB as shown is reasonable).
- Use a priority level appropriate for the application's timing requirements; 150 is a moderate priority.
- For production use, implement proper error handling and device re-detection if `sAPI_OneWireSetupReset()` fails repeatedly.
