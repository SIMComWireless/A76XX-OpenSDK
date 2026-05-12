# demo_bt

Location: `SIMCOM_SDK_SET/sc_demo/V1/src/demo_bt.c`

## Overview

This demo provides an interactive UART-driven menu for controlling the classic Bluetooth (BR/EDR) subsystem on the A7672 module. It demonstrates the legacy Bluetooth API (prefixed with `sAPI_BT*`) for device management, device discovery, pairing, and SPP (Serial Port Profile) data communication. The demo is conditionally compiled under the `BT_SUPPORT` preprocessor flag.

## Main Features

- Bluetooth device open/close lifecycle management
- MAC address get/set operations
- Bluetooth device name get/set
- Device scanning (inquiry) with configurable duration and response count
- Device pairing and unpairing with automatic IO capability negotiation
- SPP connection, disconnection, and bidirectional data transfer
- RSSI reading of connected devices
- UART-interactive menu system for manual testing

## Key Header Dependencies

- `simcom_bt.h` -- Legacy Bluetooth API header (not found as a standalone file in the SDK inc directory; likely provided through the older BT subsystem build chain). Provides `sAPI_BT*` function declarations and the old-style `struct sc_bt_*` types.
- `simcom_os.h` -- RTOS primitives (`sAPI_TaskSleep`)
- `simcom_common.h` -- Common type definitions (`UINT8`, `UINT32`)
- `simcom_debug.h` -- Debug logging (`sAPI_Debug`)
- `simcom_uart.h` -- UART I/O for interactive menu (`UartReadValue`, `UartReadLine`)
- `uart_api.c` -- Additional UART helpers (`PrintfOptionMenu`, `PrintfResp`)

## SDK APIs Observed

- `sAPI_BTOpen()` -- Opens the Bluetooth device. Returns 0 on success.
- `sAPI_BTClose()` -- Closes the Bluetooth device.
- `sAPI_BTRegisterEventHandle(bt_event_handle_cb)` -- Registers the asynchronous event callback. Must be called before `sAPI_BTOpen()`.
- `sAPI_BTSetIOCap(NO_DISPALY_AND_NO_KEYBOARD)` -- Sets the IO capability mode for pairing negotiation.
- `sAPI_BTGetIOCap(&local_io_capability)` -- Retrieves the current local IO capability.
- `sAPI_BTSetAddress(addr)` -- Sets the BT MAC address (6-byte array). Must be set before open.
- `sAPI_BTGetAddress(address.bytes)` -- Gets the current BT MAC address.
- `sAPI_BTSetName(name, len)` -- Sets the device name.
- `sAPI_BTGetName(name, &nameLen)` -- Gets the device name and its length.
- `sAPI_BTScanStart(scan_duration, scan_response_num)` -- Starts inquiry scan. Duration range: 1-48 (unit: 1.28s). Response count: >0.
- `sAPI_BTScanStop()` -- Stops an ongoing inquiry scan.
- `sAPI_BTPairStart(addr)` -- Initiates pairing with a peer device.
- `sAPI_BTPairAccept(pair_mode, addr, 0)` -- Accepts an incoming pairing request with a specified mode (JUST_WORK_MODE, NUMERIC_COMPARISON_MODE).
- `sAPI_BTUnpair(addr)` -- Removes pairing with a specific device.
- `sAPI_BTPaired(bt_device_record_array, &num)` -- Gets the list of paired devices (array of `struct sc_bt_device_record`, max 10).
- `sAPI_BTSPPConnect(addr)` -- Initiates an SPP connection to a paired device.
- `sAPI_BTSPPDisconnect(port)` -- Disconnects an SPP connection by port number.
- `sAPI_BTSPPSend(data, length, port)` -- Sends data over an SPP connection.
- `sAPI_BTReadRssi()` -- Reads the RSSI of the current connection.

## Usage

1. Build the project with `BT_SUPPORT` defined.
2. Enter the BTDemo menu (usually via the main demo dispatcher).
3. **Open**: Select option 1 to register the event callback and open BT.
4. **Set Address/Name**: Optionally configure MAC address (option 3) and device name (option 5) before or after open.
5. **Scan**: Option 7 starts inquiry; discovered devices appear in the event callback as `SC_BTTASK_IND_INQUIRY_RESULT`.
6. **Pair**: Option 9 initiates pairing; the event callback handles `SC_BTTASK_IND_PAIRING_REQUEST` and auto-accepts based on IO capability matrix.
7. **SPP Connect**: Option 12 connects SPP to a paired device address.
8. **SPP Send/Receive**: Option 14 sends data; incoming data arrives as `SC_BTTASK_IND_SPP_DATA_IND`.
9. **Close**: Option 2 closes the BT subsystem.

## Implementation Notes

- **Event-driven architecture**: All asynchronous operations (scan results, pairing, SPP events) flow through the single `bt_event_handle_cb` callback, dispatched by `msg->event_type` (SC_BTTASK_IND_TYPE_COMMON or SC_BTTASK_IND_TYPE_SPP).
- **Pairing mode matrix**: A 5x5 lookup table (`g_bt_pairing_mode`) maps local IO capability x peer IO capability to the resulting pairing mode. JUST_WORK and NUMERIC_COMPARISON are auto-accepted; PASSKEY_ENTRY is not auto-handled in this demo.
- **SPP frame size limit**: The constant `BT_SPP_MAX_FRAME_SIZE` is 200 bytes. Received data exceeding `min(bt_spp_max_frame_size, 200)` is discarded with a warning.
- **MAC address byte order**: The demo reverses byte order when displaying addresses (bytes[5]..bytes[0] for display vs bytes[0]..bytes[5] for storage). Input addresses are parsed from hex string in MSB-first order and stored in reverse.
- **Port tracking**: `bt_spp_port` and `bt_spp_max_frame_size` are tracked as static globals, updated on SPP connect/disconnect events, and reset to 0 on shutdown.

## Best Practices

- Always register the event callback before calling `sAPI_BTOpen()`.
- Set the MAC address and device name before opening BT if a custom identity is needed.
- Check the SPP port and max frame size before sending data; do not exceed the frame size limit.
- Use `sAPI_TaskSleep` with a short delay (e.g., 200ms) before auto-accepting pairing to avoid race conditions.
- For production code, handle PASSKEY_ENTRY_MODE explicitly rather than relying on auto-accept.
- Always call `sAPI_BTClose()` on shutdown and wait for the `SC_BTTASK_IND_SHUTDOWN_COMPLETE` event before reinitializing.
