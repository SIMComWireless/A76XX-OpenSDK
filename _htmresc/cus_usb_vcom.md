# cus_usb_vcom (USB Virtual COM Port)

Location: SIMCOM_SDK_SET/sc_demo/V1/src/cus_usb_vcom.c

## Overview
`cus_usb_vcom` demonstrates USB Virtual COM Port (VCOM) communication on the SIMCom module. It registers a callback for receiving data from the USB AT port, parses incoming data to distinguish between AT commands and application data, and provides the mechanism to forward data to the UI demo task.

## Main Features
- Registers a USB VCOM receive callback via `sAPI_UsbVcomRegisterCallbackEX`.
- Provides two callback variants: `UsbVcomCBFunc` (basic) and `UsbVcomCBFuncEx` (extended with known data length).
- Detects whether incoming data is an AT command (starts with "AT" or "at" and ends with "\r\n").
- Routes AT commands to the modem's AT command processor via `sAPI_AtSend`.
- Forwards non-AT application data to the UI demo task via `CacheDataToUIDemo` (when `SIMCOM_UI_DEMO_TO_USB_AT_PORT` is defined).
- Checks USB bus connection status via `sAPI_UsbVbusDetect`.

## Key Header Dependencies
- `simcom_usb_vcom.h` — USB VCOM API prototypes: `sAPI_UsbVcomRead`, `sAPI_UsbVcomWrite`, `sAPI_UsbVcomRegisterCallbackEX`, `sAPI_UsbVbusDetect`.
- `simcom_at.h` — `sAPI_AtSend` for forwarding AT commands received via USB.
- `simcom_debug.h` — `sAPI_Debug` for logging received data.
- `userspaceConfig.h` — Platform configuration macros.
- `uart_api.h` — `CacheDataToUIDemo` function for passing data to the UI demo (externally defined).

## SDK APIs Observed
- `sAPI_UsbVcomRegisterCallbackEX(UsbVcomCBFuncEx, para)` — registers the extended USB VCOM receive callback with data length parameter.
- `sAPI_UsbVcomRead(buf, maxLen)` — reads data from USB VCOM into buffer; returns the number of bytes actually read.
- `sAPI_UsbVbusDetect()` — returns 1 if USB bus is connected, 0 otherwise.
- `sAPI_AtSend(data, len)` — sends AT command string to the modem's AT command processor for execution.
- `sAPI_UsbVcomWrite(data, len)` — writes data back to USB VCOM (used in other parts of the framework).

## Usage
1. Call `sAPP_UsbVcomTask()` at startup to register the USB VCOM callback.
2. When data arrives on the USB AT port, the registered callback (`UsbVcomCBFuncEx`) is triggered.
3. The callback reads available data via `sAPI_UsbVcomRead`.
4. It checks `sAPI_UsbVbusDetect()` to confirm USB is still connected.
5. The `isATCommand()` function parses the data to determine if it starts with "AT/at" and ends with "\r\n".
6. AT commands are forwarded to the modem via `sAPI_AtSend`; other data goes to `CacheDataToUIDemo`.
7. Dynamically allocated memory is freed after processing.

## Implementation Notes
- Uses a fixed 512-byte receive buffer (`USB_VCOM_RX_BUFFER_SIZE`) for the basic callback; the extended callback allocates based on the actual received length.
- AT command detection uses `strncmp` for the "AT"/"at" prefix and `strstr` for the "\r\n" tail.
- The extended callback (`UsbVcomCBFuncEx`) is the default registration; the basic callback (`UsbVcomCBFunc`) is provided as an alternative.
- The `SIMCOM_UI_DEMO_TO_USB_AT_PORT` preprocessor macro controls whether non-AT data is forwarded to the UI demo.
- Memory is allocated with `malloc` and freed with `free` in the callback.

## Best Practices
- Always loop-read in the callback until `sAPI_UsbVcomRead` returns 0 to handle multiple packets in a single callback invocation.
- Validate `sAPI_UsbVbusDetect()` before processing data, as the USB connection may have been disconnected.
- In production, implement a proper data framing protocol instead of relying solely on AT command detection.
- Avoid long-running operations inside the USB VCOM callback; forward data to a processing task instead.
- Consider using the extended callback (`UsbVcomCBFuncEx`) for better memory efficiency since it knows the exact data length.
